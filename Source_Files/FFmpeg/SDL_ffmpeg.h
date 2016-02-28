/*******************************************************************************
*                                                                              *
*   SDL_ffmpeg is a library for basic multimedia functionality.                *
*   SDL_ffmpeg is based on ffmpeg.                                             *
*                                                                              *
*   Copyright (C) 2007  Arjan Houben                                           *
*                                                                              *
*   SDL_ffmpeg is free software: you can redistribute it and/or modify         *
*   it under the terms of the GNU Lesser General Public License as published   *
*   by the Free Software Foundation, either version 3 of the License, or any   *
*   later version.                                                             *
*                                                                              *
*   This program is distributed in the hope that it will be useful,            *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               *
*   GNU Lesser General Public License for more details.                        *
*                                                                              *
*   You should have received a copy of the GNU Lesser General Public License   *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
*                                                                              *
*******************************************************************************/

#ifndef SDL_FFMPEG_INCLUDED
#define SDL_FFMPEG_INCLUDED

#include "SDL_thread.h"
#include "SDL.h"

#ifdef __cplusplus
extern "C" {
#endif

// #ifdef WIN32
//     #ifdef BUILD_DLL
//         #define EXPORT __declspec(dllexport)
//     #else
//         #define EXPORT __declspec(dllimport)
//     #endif
// #else
    #define EXPORT
// #endif

enum SDL_ffmpegStreamType
{
    SDL_ffmpegUninitialized = 0,
    SDL_ffmpegOutputStream,
    SDL_ffmpegInputStream
};

typedef void (*SDL_ffmpegCallback)(void *userdata, Uint8 *stream, int len);

typedef struct SDL_ffmpegConversionContext
{
    int inWidth, inHeight, inFormat,
    outWidth, outHeight, outFormat;

    struct SwsContext *context;

    struct SDL_ffmpegConversionContext *next;
} SDL_ffmpegConversionContext;

/** Struct to hold codec values */
typedef struct
{
    /** video codec ID */
    int32_t videoCodecID;
    /** width of the output stream */
    int32_t width;
    /** height of the output stream */
    int32_t height;
    /** numinator part of the framerate */
    int32_t framerateNum;
    /** denominator part of the framerate */
    int32_t framerateDen;
    /** bitrate of video stream in bytes */
    int32_t videoBitrate;
    /** when variable bitrate is desired, this holds the minimal video bitrate */
    int32_t videoMinRate;
    /** when variable bitrate is desired, this holds the maximal video bitrate */
    int32_t videoMaxRate;
    /** audio codec ID */
    int32_t audioCodecID;
    /** number of audio channels in stream */
    int32_t channels;
    /** audio samplerate of output stream */
    int32_t sampleRate;
    /** bitrate of audio stream in bytes */
    int32_t audioBitrate;
    /** when variable bitrate is desired, this holds the minimal audio bitrate */
    int32_t audioMinRate;
    /** when variable bitrate is desired, this holds the maximal audio bitrate */
    int32_t audiooMaxRate;
} SDL_ffmpegCodec;

/** predefined codec for PAL DVD */
EXPORT extern const SDL_ffmpegCodec SDL_ffmpegCodecPALDVD;

/** predefined codec for DV */
EXPORT extern const SDL_ffmpegCodec SDL_ffmpegCodecPALDV;

/** predefined codec based on extension of output file */
EXPORT extern const SDL_ffmpegCodec SDL_ffmpegCodecAUTO;

/** Struct to hold packet buffers */
typedef struct SDL_ffmpegPacket {
    struct AVPacket *data;
    struct SDL_ffmpegPacket *next;
} SDL_ffmpegPacket;

/** Struct to hold audio data */
typedef struct
{
    /** Presentation timestamp, time at which this data should be used. */
    int64_t pts;
    /** Pointer to audio buffer, user adjustable. */
    uint8_t *buffer;
    /** Size of this audio frame. */
    uint32_t size;
    /** Size of the complete audio frame. */
    uint32_t capacity;
	/** Value indicating wheter or not this is the last frame before EOF */
	int last;
} SDL_ffmpegAudioFrame;


/** Struct to hold audio data */
typedef struct
{
    /** Presentation timestamp, time at which this data should be used. */
    int64_t pts;
    /** Pointer to video buffer, user adjustable. */
    SDL_Surface *surface;
    /** Pointer to overlay buffer, user adjustable. */
//    SDL_Overlay *overlay;
    /** Value indicating if this frame holds data, or that it can be overwritten. */
    int ready;
	/** Value indicating wheter or not this is the last frame before EOF */
	int last;
} SDL_ffmpegVideoFrame;

/** This is the basic stream for SDL_ffmpeg */
typedef struct SDL_ffmpegStream
{
    /** Pointer to ffmpeg data, internal use only! */
    struct AVStream *_ffmpeg;

    /** Intermediate frame which will be used when decoding */
    struct AVFrame *decodeFrame;
    /** Intermediate frame which will be used when encoding */
    struct AVFrame *encodeFrame;
    /** Store conversion context for this stream */
    struct SDL_ffmpegConversionContext *conversionContext;

    int encodeFrameBufferSize;
    uint8_t *encodeFrameBuffer;

    int encodeAudioInputSize;
    uint64_t frameCount;

    /** buffer for decoded audio data */
    int8_t *sampleBuffer;
    /** amount of data in samplebuffer */
    int sampleBufferSize;
    /** position of data in samplebuffer */
    int sampleBufferOffset;
    /** stride of planar data in samplebuffer */
    int sampleBufferStride;
    /** timestamp which fits the data in samplebuffer */
    int64_t sampleBufferTime;

    /** packet buffer */
    SDL_ffmpegPacket *buffer;
    /** mutex for multi threaded acces to buffer */
    SDL_mutex *mutex;

    /** Id of the stream */
    int id;
    /** This holds the lastTimeStamp calculated, usefull when frames don't provide
        a usefull dts/pts, also used for determining at what point we are in the file */
    int64_t lastTimeStamp;

    /** pointer to the next stream, or NULL if current stream is the last one */
    struct SDL_ffmpegStream *next;
} SDL_ffmpegStream;

/** Struct to hold information about file */
typedef struct
{
    /** type of file */
    enum SDL_ffmpegStreamType type;

    /** Pointer to ffmpeg data, internal use only! */
    struct AVFormatContext *_ffmpeg;

    /** Video streams */
    SDL_ffmpegStream    *vs,
    /** Audio streams */
                        *as;

    /** stream mutex */
    SDL_mutex           *streamMutex;

    /** Amount of video streams in file */
    uint32_t            videoStreams,
    /** Amount of audio streams in file */
                        audioStreams;

    /** Pointer to active videoStream, NULL if no video stream is active */
    SDL_ffmpegStream    *videoStream,
    /** Pointer to active audioStream, NULL if no audio stream is active */
                        *audioStream;

    /** Holds the lowest timestamp which will be decoded */
    int64_t             minimalTimestamp;
} SDL_ffmpegFile;

/* error handling */
EXPORT const char* SDL_ffmpegGetError();

EXPORT void SDL_ffmpegClearError();

/* SDL_ffmpegFile create / destroy */
EXPORT SDL_ffmpegFile* SDL_ffmpegOpen( const char* filename );

#ifdef SDL_FF_WRITE
EXPORT SDL_ffmpegFile* SDL_ffmpegCreate( const char* filename );
#endif

EXPORT void SDL_ffmpegFree( SDL_ffmpegFile* file );

/* general */
EXPORT int SDL_ffmpegSeek( SDL_ffmpegFile* file, uint64_t timestamp );

EXPORT int SDL_ffmpegSeekRelative( SDL_ffmpegFile* file, int64_t timestamp );

EXPORT uint64_t SDL_ffmpegDuration( SDL_ffmpegFile *file );

EXPORT int64_t SDL_ffmpegGetPosition( SDL_ffmpegFile *file );

EXPORT float SDL_ffmpegGetFrameRate( SDL_ffmpegStream *stream, int *numerator, int *denominator );

/* video stream */
#ifdef SDL_FF_WRITE
EXPORT SDL_ffmpegStream* SDL_ffmpegAddVideoStream( SDL_ffmpegFile *file, SDL_ffmpegCodec );
#endif

EXPORT SDL_ffmpegStream* SDL_ffmpegGetVideoStream( SDL_ffmpegFile *file, uint32_t videoID );

EXPORT int SDL_ffmpegSelectVideoStream( SDL_ffmpegFile* file, int videoID);

/* video frame */
EXPORT SDL_ffmpegVideoFrame* SDL_ffmpegCreateVideoFrame();

#ifdef SDL_FF_WRITE
EXPORT int SDL_ffmpegAddVideoFrame( SDL_ffmpegFile *file, SDL_Surface *frame );
#endif

EXPORT int SDL_ffmpegGetVideoFrame( SDL_ffmpegFile *file, SDL_ffmpegVideoFrame *frame );

EXPORT void SDL_ffmpegFreeVideoFrame( SDL_ffmpegVideoFrame* frame );

/* video specs */
EXPORT int SDL_ffmpegGetVideoSize( SDL_ffmpegFile *file, int *w, int *h);

/* general video */
EXPORT int SDL_ffmpegValidVideo( SDL_ffmpegFile *file );

EXPORT uint64_t SDL_ffmpegVideoDuration( SDL_ffmpegFile *file );


/* audio stream */
#ifdef SDL_FF_WRITE
EXPORT SDL_ffmpegStream* SDL_ffmpegAddAudioStream( SDL_ffmpegFile *file, SDL_ffmpegCodec );
#endif

EXPORT SDL_ffmpegStream* SDL_ffmpegGetAudioStream( SDL_ffmpegFile *file, uint32_t audioID);

EXPORT int SDL_ffmpegSelectAudioStream( SDL_ffmpegFile* file, int audioID);

/* audio frame */
EXPORT SDL_ffmpegAudioFrame* SDL_ffmpegCreateAudioFrame( SDL_ffmpegFile *file, uint32_t bytes );

#ifdef SDL_FF_WRITE
EXPORT int SDL_ffmpegAddAudioFrame( SDL_ffmpegFile *file, SDL_ffmpegAudioFrame *frame );
#endif

EXPORT int SDL_ffmpegGetAudioFrame( SDL_ffmpegFile *file, SDL_ffmpegAudioFrame *frame );

EXPORT void SDL_ffmpegFreeAudioFrame( SDL_ffmpegAudioFrame* frame );

/* audio specs */
EXPORT SDL_AudioSpec SDL_ffmpegGetAudioSpec( SDL_ffmpegFile *file, uint16_t samples, SDL_ffmpegCallback callback );

/* general audio */
EXPORT int SDL_ffmpegValidAudio( SDL_ffmpegFile *file );

EXPORT uint64_t SDL_ffmpegAudioDuration( SDL_ffmpegFile *file );

/** \cond */

/* these functions are not public */
#ifdef SDL_FF_WRITE
EXPORT SDL_ffmpegFile* SDL_ffmpegCreateFile();
#endif

EXPORT int SDL_ffmpegFlush( SDL_ffmpegFile *file );

/** \endcond */

#ifdef __cplusplus
}
#endif

#endif /* SDL_FFMPEG_INCLUDED */
