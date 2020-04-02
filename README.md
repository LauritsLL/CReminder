## CReminder - A Reminder to help you remember tasks!

(This text is all a joke - Please don't take it seriously)

Have you ever had trouble remembering to do your homework, take your medicin, or coding that application that your boss have been asking for, for a very long time?
Well, look no further - I present to you, the CReminder!

### Installation and Running the Program

1. Clone this repository.
`git clone https://github.com/Lause132/CReminder`
2. Install your prefered [C Compiler](#inst-c-compiler) (`gcc`, `clang`, ..)

To run this program you compile the program with the `-lpthread` flag to support multi-threading as this program is using that.
Example with the GNU GCC Compiler:
`gcc reminder.c -Wall -Wextra -g -o reminder -lpthread`

### Installing a C Compiler (GCC) <a name="inst-c-compiler"></a>

To install GCC, you'll need to install either [cygwin](https://cygwin.com/install.html) or [mingw](https://osdn.net/projects/mingw/releases/) to emulate a linux environment, and choose the 64-bit or 32-bit version based on, if your operating system is 64-bit or 32-bit.
> I recommend Cygwin, as I can confirm that it's easy to install and works great for me!

**NOTE:** If you install Cygwin, then remember to check GCC and GDB for installation when you install extra packages.
