# Aleph One — VR render integration map (Phase 2 prep)

Where stereo rendering and head tracking graft into Aleph One's render path. Pairs with the engine↔VR
contract in [VR_PORT_PLAN.md](VR_PORT_PLAN.md) §3.1. **No code changed by this doc** — it's the survey
that makes Phase 2 a known, localized surface (and confirms Phase 1 feeds directly into it).

---

## 1. The render frame pipeline

Entry: `render_view(view_data* view, bitmap_definition* sw_dest)` in `Source_Files/RenderMain/render.cpp:420`.
Per frame, for the 3D world (non-terminal, non-overhead) it does:

| Step | Code | Eye-dependent? |
|---|---|---|
| Update view (FOV lerp, derived fields) | `update_view_data(view)` (`render.cpp:424`) | no |
| Build visibility tree (portal/cone clip windows) | `RenderVisTree.build_render_tree()` (`:450`) | **screen-space** ⚠ |
| Depth-sort polygons + accumulate clip windows | `RenderSortPoly.sort_render_tree()` (`:459`) | screen-space ⚠ |
| Place sprites/objects | `RenderPlaceObjs.build_render_object_list()` (`:464`) | mostly no |
| **Set projection + modelview** | `RasPtr->SetView(*view)` (`:482`) | **YES — the seam** |
| Begin (activate FBO swapper) | `RasPtr->Begin()` (`:485`) | per-eye target |
| Draw world | `RenPtr->render_tree()` (`:497`) | uses the matrices |
| Draw weapon-in-hand | `render_viewer_sprite_layer(...)` (`:502`) | uses the matrices |
| End (resolve swapper, gamma, **blit to screen**) | `RasPtr->End()` (`:506`) | **blits to screen ⚠** |

`RasPtr` is `Rasterizer_Shader` when `OGL_IsActive()` (`render.cpp:469-470`), else the software rasterizer.

---

## 2. The seam: `Rasterizer_Shader_Class::SetView`

`Source_Files/RenderMain/Rasterizer_Shader.cpp:52-128`. This one function builds **both** matrices, and
it is exactly the fixed-function code Phase 1 must rewrite anyway:

**Projection** (`:81-88`):
```cpp
glMatrixMode(GL_PROJECTION); glLoadIdentity();
float nearVal = 64.0, farVal = 128.0*1024.0;   // Marathon world units
float x = xtan*nearVal, y = ytan*nearVal;       // xtan/ytan from view.field_of_view + aspect
glFrustum(-x, x, -y+yoff, y+yoff, nearVal, farVal);
```
**Modelview** (`:90-127`), after uploading a landscape-inverse matrix to the sky shaders:
```cpp
glLoadMatrixd(kViewBaseMatrix);                 // Marathon (Z-up) → GL eye axis remap
if (!view.mimic_sw_perspective) glRotated(pitch, 0,1,0);
glRotated(-yaw, 0,0,1);
glTranslated(-view.origin.x, -view.origin.y, -view.origin.z);
```
`kViewBaseMatrix` / `kViewBaseMatrixInverse` (`:35-47`) are the world↔eye axis remap (Marathon world is
Z-up). Keep these — OpenXR poses (Y-up, metres) compose *outside* this remap.

### `view_data` fields this reads (`render.h:87-144`)
`field_of_view`, `screen_width/height` (→ aspect), `virtual_yaw`, `virtual_pitch`, `origin` (world_point3d),
`mimic_sw_perspective`, `real_world_to_screen_*` (teleport distortion). Phase 2 repurposes yaw/pitch
(see §4).

---

## 3. What Phase 1 turns this into

Phase 1 (GLES modernization) removes the FF matrix stack, so `SetView` becomes:
```cpp
Mat4 projection = perspectiveFrustum(-x,x,-y+yoff,y+yoff, nearVal, farVal);   // computed in C++
Mat4 modelview  = baseRemap * rotZ(-yaw) * [rotY(pitch)] * translate(-origin); // computed in C++
// store both; upload as uProjection / uModelview (or uMVP) to every world shader
```
Every draw site in `RenderRasterize_Shader.cpp` then references these uniforms instead of the FF state.
**This is the enabling step for Phase 2** — once the matrices are explicit values rather than GL state,
swapping in VR matrices is a one-line substitution.

---

## 4. What Phase 2 substitutes

Make `SetView` eye-aware: `SetView(view_data& view, int eye)`.

**Projection ← OpenXR** (replaces `:81-88` entirely):
```cpp
float proj[16];
VR_GetVRProjection(eye, /*zNear*/64.0f, /*zFar*/128.0f*1024.0f, proj);  // off-axis frustum from eye fov
projection = Mat4(proj);
```

**Modelview ← per-eye head pose composed with the body transform** (replaces `:119-127`):
```cpp
modelview = eyeView(eye)            // = inverse(stageFromEye); from TBXR, Y-up metres
          * metresToWorldUnits      // world-scale (see risk below)
          * baseRemap               // kViewBaseMatrix (Marathon Z-up remap)
          * rotZ(-bodyYaw)          // locomotion / snap-turn ONLY (NOT head yaw)
          * translate(-origin);     // player body position in world units
```
Key split: **head orientation + positional offset come from `eyeView(eye)`**, so we must *stop* applying
the mouse-look `view.virtual_yaw/virtual_pitch` to the camera. Instead:
- `bodyYaw` = stick/snap-turn yaw fed in via the `VR_GetMove()`-style accessor (plan §3.1).
- Head yaw/pitch/roll and the in-play-space position delta are baked into `eyeView`.
- `view.origin` becomes the player's *body* position; the HMD position offset rides in through `eyeView`.

The landscape-inverse matrix (`:92-113`) feeding the sky shaders must use the HMD yaw/pitch too, else the
skybox won't track head rotation.

---

## 5. The per-eye loop (in `render_view`)

Build the scene **once** from the head/center pose, render it **twice**:
```cpp
// once: update_view_data + build_render_tree + sort + place   (render.cpp:424-464)
for (int eye = 0; eye < 2; ++eye) {
    TBXR_prepareEyeBuffer(eye);          // binds + clears the eye FBO
    RasPtr->SetView(*view, eye);         // VR projection + per-eye modelview (§4)
    RasPtr->Begin();
    RenPtr->render_tree();
    render_viewer_sprite_layer(view, RasPtr);
    RasPtr->End();                       // must resolve into the eye FBO, NOT blit to screen (§6)
    TBXR_finishEyeBuffer(eye);
}
// caller: TBXR_FrameSetup() before, TBXR_submitFrame() after the loop
```

---

## 6. Risks / subtleties surfaced by this map

1. **Screen-space visibility (⚠ biggest).** `RenderVisTree` builds portal **clip windows in screen
   columns** from the 2D view cone, and `RenderSortPoly` accumulates per-polygon clip windows the same
   way (these are the `glClipPlane` users from plan §2.1). For VR the cone is per-eye and very wide; the
   tree must be built with a cone wide enough to cover both eyes, or the clipping reworked. This is the
   same screen-space assumption flagged in plan §5 risk 2/3 — Phase 2's hardest sub-problem.
2. **`End()` blits to screen.** `Rasterizer_Shader_Class::End()` (`Rasterizer_Shader.cpp:150-169`)
   resolves its internal `FBOSwapper`, applies gamma, then `OGL_RenderFrame(...)` blits to the default
   framebuffer. In VR each eye must land in the TBXR eye FBO that `TBXR_prepareEyeBuffer` bound — needs an
   eye-aware End() (size the swapper to eye resolution; final draw targets the bound FBO).
3. **World scale (WU↔metres).** OpenXR is metric; Marathon is world units (`WORLD_ONE = 1024`). Must fix a
   scale so the player feels human-sized. Affects `metresToWorldUnits` and comfort.
4. **FOV source.** Drop `view.field_of_view` for the world projection (still used for weapon sprite layout
   / HUD); the per-eye frustum comes from OpenXR's `angleLeft/Right/Up/Down`.
5. **Don't double-apply look.** Ensure mouse/stick pitch isn't added on top of the HMD pitch.

---

## 7. Files Phase 2 will touch (summary)

- `Source_Files/RenderMain/Rasterizer_Shader.cpp` — `SetView` → eye-aware (projection + modelview).
- `Source_Files/RenderMain/Rasterizer_Shader.h` / `Rasterizer.h` — `SetView(view, eye)` signature.
- `Source_Files/RenderMain/render.cpp` — per-eye loop in `render_view` (§5), build-once/render-twice.
- `Source_Files/RenderMain/Rasterizer_Shader.cpp` — eye-aware `End()` / FBO target.
- `RenderVisTree.*` / `RenderSortPoly.*` — wide-cone or reworked clip windows (risk 1).
- New `AlephOne_OpenXR.cpp` — implements `VR_GetVRProjection`, head-pose globals, `VR_GetMove`.
