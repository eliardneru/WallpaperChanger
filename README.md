<img width="128" height="128" alt="graphicdesignismypassion" src="https://github.com/user-attachments/assets/6d4bf480-bf14-4f33-9e16-e098393fdf2a" /> 

# WallpaperChanger

a simple gui software that hanges your wallpaper given a certain period of time


# installation
just unzip the file and run the .exe, if you want it to run at the windows startup you have to copy a shortcut of it to shell::startup

# features
- shuffling wallpapers randomly
- change wallpaper every period of time
- exit to tray
- a cool timer that shows when the next wallpaper is changing
- counter of how many wallpapers are loaded
- toggle to change wallpaper on program startup

# Building and compiling
good luck, you are on your own
jk, but i use QT Creator to compile it, i'm not sure how you would do it without it

# FAQ
## why you made this and didn't just use other software such as John's Background Switcher?
too much bloat and didn't have the features i actually wanted, such as shuffling
## why can't i change wallpapers more than every 99 hours and 66 minutes?
this is due to QTimer's limit, but also because i cannot see a good reason to why one would want to go over that, like what is wrong with you
## will this waste my ram?
it uses 4mb of ram last time i checked, i did have some troubles with memory leak's in other wallpaper switchers such as john's
## does this has bugs?
definitely
## will you update this?
not sure, it has enough features for my personal usage
## why is everything in main?
i don't like headers
