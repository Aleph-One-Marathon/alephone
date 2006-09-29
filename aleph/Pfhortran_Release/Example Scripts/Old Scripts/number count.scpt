total: def 0
tag: def 0
set_tag_state 1, FALSE
top: wait_ticks 150
get_tag_state 1, tag
if_equal tag, FALSE, next
inflict_damage total
set_tag_state 1, FALSE
next: add total, 1
jump dim
check: if_equal total, 10, ten
if_equal total, 0, zero
if_equal total, 1, one
if_equal total, 2, two
if_equal total, 3, three
if_equal total, 4, four
if_equal total, 5, five
if_equal total, 6, six
if_equal total, 7, seven
if_equal total, 8, eight
if_equal total, 9, nine
set total, 0
jump top
dim: set_tag_state 9, FALSE
set_tag_state 10, FALSE
set_tag_state 11, FALSE
set_tag_state 12, FALSE
set_tag_state 13, FALSE
set_tag_state 14, FALSE
set_tag_state 15, FALSE
jump check
zero: set_tag_state 9, TRUE
set_tag_state 10, TRUE
set_tag_state 12, TRUE
set_tag_state 13, TRUE
set_tag_state 14, TRUE
set_tag_state 15, TRUE
jump top
one: set_tag_state 10, TRUE
set_tag_state 12, TRUE
jump top
two: set_tag_state 9, TRUE
set_tag_state 11, TRUE
set_tag_state 12, TRUE
set_tag_state 13, TRUE
set_tag_state 15, TRUE
jump top
three: set_tag_state 9, TRUE
set_tag_state 10, TRUE
set_tag_state 11, TRUE
set_tag_state 12, TRUE
set_tag_state 13, TRUE
jump top
four: set_tag_state 10, TRUE
set_tag_state 11, TRUE
set_tag_state 12, TRUE
set_tag_state 14, TRUE
jump top
five: set_tag_state 9, TRUE
set_tag_state 10, TRUE
set_tag_state 11, TRUE
set_tag_state 13, TRUE
set_tag_state 14, TRUE
jump top
six: set_tag_state 9, TRUE
set_tag_state 10, TRUE
set_tag_state 11, TRUE
set_tag_state 14, TRUE
set_tag_state 15, TRUE
jump top
seven: set_tag_state 10, TRUE
set_tag_state 12, TRUE
set_tag_state 13, TRUE
jump top
eight: set_tag_state 9, TRUE
set_tag_state 10, TRUE
set_tag_state 11, TRUE
set_tag_state 12, TRUE
set_tag_state 13, TRUE
set_tag_state 14, TRUE
set_tag_state 15, TRUE
jump top
nine: set_tag_state 10, TRUE
set_tag_state 11, TRUE
set_tag_state 12, TRUE
set_tag_state 13, TRUE
set_tag_state 14, TRUE
jump top
ten: set total, 0
jump check
