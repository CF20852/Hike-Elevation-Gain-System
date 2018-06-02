# Arduino Library FAQ
## Where does the Arduino IDE look for libraries?
The Arduino IDE looks for libraries in several places (the following is Windows specific):  (1) the libraries folder in your sketchbook, e.g. C:\Users\username\My Documents\Arduino\libraries; (2) in the libraries folder of the Arduino IDE installation folder, e.g. C:\Program Files (x86)\Arduino\libraries; (3) in the folders below C:\Program Files (x86)\Arduino\hardware, e.g., C:\Program Files (x86)\Arduino\hardware\arduino\avr\libraries; and (4) in your sketch folder.  If you want to include a library in one of the first three places, you *should* enclose the appropriate header file name in angle brackets (<>), e.g.,

`#include <SomeLib.h>`, 

and if you want to include a library in your sketch (not sketchbook) folder, or in a library folder inside a src folder in your sketch folder, you *must* enclose the header file name in double quotes, e.g.

`#include "MyLib.h"` or<br>
`#include "src/MyLib1/src/MyLib1.h"`.

Note that if you use the double-quote syntax, the Arduino IDE will also look in the places listed above for the angle brackets syntax if it doesn't find the source files in your sketch folder.

## What do I do if I need multiple versions of the same library, or a customized version?
First, some cautionary advice:  The Arduino IDE will generate advisory (not error, or warning) messages if it finds multiple, identically-named copies of the same header files in the locations in which it searches for libraries.  This should tell you to think carefully about what you've done.  

Moreover, and more importantly, if you update the library using the Arduino Library Manager, the new library folder will replace the existing library folder with the same name in your sketchbook folder.  If you update a library using the Arduino Library Manager while the Arduino IDE sketchbook folder is set to your specific sketchbook location, the library in your specific sketchbook location will be overwritten. In the interest of caution and at the cost of repetition, I'll re-emphasize this point below.

Four options you can consider are:

1.  Under this option, your libraries are assumed to be in the default library location, which under Windows, is C:\Users\username\My Documents\Arduino\libraries.  If there are a small number of source code (i.e., .h and .cpp) files in the library, you can assign different file names to different versions of the source code files.  *If you choose this option, be sure to back up your custom source code files if you decide to update the library.  If you update a library using the Arduino Library Manager while the Arduino IDE sketchbook folder is set to the default sketchbook location, the library in the default sketchbook location will be overwritten.*
2.  If there are many source code files in the library, it might be better to place your sketch and the custom libraries it requires, along with any other libraries your sketch requires that are normally located in the default sketchbook folder, in a separate sketchbook folder.  You can tell the Arduino IDE to use that sketchbook folder by going to File... Preferences... Sketchbook location in the Arduino IDE and setting the sketchbook location to the appropriate folder.  You should maintain the structure of the original library folder(s), including maintaining the source code in a subfolder of your library folder(s) called "src".  You may also need to tailor the example sketches for your customized libraries according to your own needs, in which case the place to do it is within the examples folder of your library folder(s).  *Be aware that if you update a library using the Arduino Library Manager while the Arduino IDE sketchbook folder is set to your specific sketchbook location, the library in your specific sketchbook location will be overwritten.*  The sketchbook structure should end up looking something like this, assuming you have a folder in the root directory of your C: drive called Arduino Sketchbooks:
<br>C:
<br>&emsp;|_Arduino Sketchbooks
<br>&emsp;&emsp;|_MySketchbook1
<br>&emsp;&emsp;&emsp;|_MySketch1
<br>&emsp;&emsp;&emsp;&emsp;|_MySketch1.ino
<br>&emsp;&emsp;&emsp;&emsp;|_libraries
<br>&emsp;&emsp;&emsp;&emsp;&emsp;|_MyLibrary1
<br>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;|_src
<br>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;|_lib1.h
<br>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;|_lib1.cpp
<br>&emsp;&emsp;&emsp;&emsp;&emsp;|_MyLibrary2
<br>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;|_src
<br>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;|_lib2.h
<br>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;|_lib2.cpp

The next two approaches were suggested by per1234:

3.  Rather than dealing with multiple sketchbooks, another way to handle the different versions of a library is to bundle a modified version of that library with each sketch. So the sketch directory tree would look like this:
<br>C:
<br>&emsp;|_Arduino Sketchbooks
<br>&emsp;&emsp;|_MySketch1
<br>&emsp;&emsp;&emsp;|_MySketch1.ino
<br>&emsp;&emsp;&emsp;|_src
<br>&emsp;&emsp;&emsp;&emsp;|_MyLibrary1
<br>&emsp;&emsp;&emsp;&emsp;&emsp;|_src
<br>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;|_lib1.h
<br>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;|_lib1.cpp
<br>&emsp;&emsp;&emsp;&emsp;|_MyLibrary2
<br>&emsp;&emsp;&emsp;&emsp;&emsp;|_src
<br>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;|_lib2.h
<br>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;|_lib2.cpp

And the sketch includes the libraries like this:
    
    #include "src/MyLibrary1/src/lib1.h"
    #include "src/MyLibrary2/src/lib2.h"

Then the sketches can all share the other libraries that don't require any modifications.

4.  Another option is to make each version of each library for which you need a hardware-specific variation a separate library in the main libraries folder (e.g., C:\Users\username\My Documents\Arduino\libraries) with a separate name. If you change the folder name then there is no problem with Library Manager updates overwriting the modified library. If you change the header file name then you will be able to control which library is included by your code. The advantage of this is that it would permit you to merge all of your sketch variations into one. The you can have a configuration macro at the top of the sketch:


`// Uncomment one of the following lines to configure the sketch:`<br>
`// #define HARDWARE_CONFIGURATION_1`<br>
`// #define HARDWARE_CONFIGURATION_2`<br>
`// #define HARDWARE_CONFIGURATION_3`<br>

Then anywhere there is code that differs between the versions:

    #if defined(HARDWARE_CONFIGURATION_1)
    // HARDWARE_CONFIGURATION_1 specific code here
    #elif defined(HARDWARE_CONFIGURATION_2)
    // HARDWARE_CONFIGURATION_2 specific code here
    #else
    // HARDWARE_CONFIGURATION_3 specific code here
    #endif

This is a good option if you have a lot of code that's shared between the various n sketches, otherwise when you want to make a change it get's really tiresome having to do it nX every time. The disadvantage is all that preprocessor code gets pretty messy.


## What do I do if I need to create my own library?
Even if you don't plan to publish your library for use by others, you should follow the [Arduino Library Specification](https://github.com/arduino/Arduino/wiki/Arduino-IDE-1.5:-Library-specification).  If you aren't going to publish your library, you don't need the library.properties file or the keywords.txt file, nor do you need to have an examples folder or an extras folder.  
