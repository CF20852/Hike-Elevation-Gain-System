# Arduino Library FAQ
## Where does the Arduino IDE look for libraries?
The Arduino IDE looks for libraries in several places (the following is Windows specific):  (1) the libraries folder in your sketchbook, e.g. C:\Users\username\My Documents\Arduino\libraries; (2) in the libraries folder of the Arduino IDE installation folder, e.g. C:\Program Files (x86)\Arduino\libraries; (3) in the folders below C:\Program Files (x86)\Arduino\hardware, e.g., C:\Program Files (x86)\Arduino\hardware\arduino\avr\libraries; and (4) in your sketch folder.  If you want to include a library in one of the first three places, you should enclose the appropriate header file name in angle brackets (<>), e.g.,

`#include <SomeLib.h>`, 

and if you want to include a library in your sketch (not sketchbook) folder, you must enclose the header file name in double quotes, e.g.

`#include "MyLib.h"`.

Note that if you use the double-quote syntax, the Arduino IDE will also look in the places listed above for the angle brackets syntax.

## What do I do if I need multiple versions of the same library?
First, a couple of messages of caution:  The Arduino IDE will generate error messages if it finds multiple, identically-named copies of the same header files in the locations in which it searches for libraries.  Moreover, if you update the library, the new library folder will replace the existing library folder in your sketchbook folder.  If you update a library using the Arduino Library Manager while the Arduino IDE sketchbook folder is set to your specific sketchbook location, the library in your specific sketchbook location will be overwritten. In the interest of caution and at the cost of repetition, I'll re-emphasize this point below.  Two options you might consider are:
1.  Under this option, your libraries are assumed to be in the default library location, which under Windows, is C:\Users\username\My Documents\Arduino\libraries.  If there are a small number of source code (i.e., .h and .cpp) files in the library, you can assign different file names to different versions of the source code files.  *If you choose this option, be sure to back up your custom source code files if you decide to update the library.  If you update a library using the Arduino Library Manager while the Arduino IDE sketchbook folder is set to the default sketchbook location, the library in the default sketchbook location will be overwritten.*
2.  If there are many source code files in the library, it might be better to place your sketch and the custom libraries it requires, along with any other libraries that are normally located in the default sketchbook folder, in a separate sketchbook folder.  You can tell the Arduino IDE to use that sketchbook folder by going to File... Preferences... Sketchbook location and setting the sketchbook location to the appropriate folder.  *Be aware that if you update a library using the Arduino Library Manager while the Arduino IDE sketchbook folder is set to your specific sketchbook location, the library in your specific sketchbook location will be overwritten.*
