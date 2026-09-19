# this is a terminal-based code editor, that i am making
- full docu coming once its done, current one may not be so good and some features aren't implemented yet








# controls
- ```Shift + Tab``` enters/exits super mode
- ```alt + f``` opens/closes filesys in the side (automatically switches to filesys mode when opened)
- ```alt + g``` enters/exits filesys mode (only if filesys is open)
- ```alt + r``` rename currently open file

## super mode
### (all super mode keys can also be used by crtl+key)
- ```w/a/s/d``` = move cursor
- ```q / e``` = set start/end of selection
- ```double q/e``` go to start/end of current line
- ```r``` reset current selection
- ```c``` copy selected text to the internal clipboard
- ```v``` paste text from the internal clipbaord
- ```x``` copy and delete selected text to the internal clipboard
> (the editor has an internal clipboard, to access the real clipbaord you can use ```shift + crtl + c``` and ```shift + crtl + v``` like always)
- ```f``` find and select all occurrences of selected symbol for renaming (by pressing the key mutliple times you can change the range of the search)
    - 1x: in current scope
    - 2x: in current file
    - 3x: in working directory


# filesys
- w / s / arrow keys = move up/down filetree
- e / enter = open file and save current one