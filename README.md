# B.Low

B.Low is the unique sample-based sound generator plugin you always waited for.
It blows out low sounds from below to spice up your music production with a 
special flavour. The high quality samples were gratefully provided by 
numerous international artists (see b-low).


## Installation

a) Install the blow package for your system (once established)

b) Use the latest provided binaries

Unpack the provided blow-\*.zip or blow-\*.tar.xz from the latest release and 
copy the BLow.lv2 folder to your lv2 directory (depending on your system settings,
~/.lv2/, /usr/lib/lv2/, /usr/local/lib/lv2/, or ...).

c) Build your own binaries in the following three steps.

Step 1: [Download the latest published version](https://github.com/sjaehn/BLow/releases) of Low. Or clone or
[download the master](https://github.com/sjaehn/BLow/archive/master.zip) of this repository.

Step 2: Install pkg-config and the development packages for x11, cairo, soundfile, and lv2 if not done yet. If you
don't have already got the build tools (compilers, make, libraries) then install them too.

On Debian-based systems you may run:
```
sudo apt-get install build-essential
sudo apt-get install pkg-config libx11-dev libcairo2-dev libsndfile-dev lv2-dev
```

On Arch-based systems you may run:
```
sudo pacman -S base-devel
sudo pacman -S pkg-config libx11 cairo libsndfile lv2
```

Step 3: Building and installing into the default lv2 directory (/usr/local/lib/lv2/) is easy using `make` and
`make install`. Simply call:
```
make
sudo make install
```

**Optional:** Standard `make` and `make install` parameters are supported. You may build a debugging version 
using `make CXXFLAGS+=-g`. For installation into an alternative directory (e.g., /usr/lib/lv2/), change the
variable `PREFIX` while installing: `sudo make install PREFIX=/usr`. If you want to freely choose the
install target directory, change the variable `LV2DIR` (e.g., `make install LV2DIR=~/.lv2`).


## Running

After the installation Ardour, Carla, and any other LV2 host should automatically detect B.Low.

If jalv is installed, you can also call it using one of the graphical jalv executables (like
jalv.gtk, or jalv.gtk3, or jalv.qt4, or jalv.qt5, depending on what is installed), like

```
jalv.gtk https://www.jahnichen.de/plugins/lv2/BLow
```

to run it stand-alone and connect it to the JACK system.


## Acknowledgments

Samples provided by:
* unfa (https://freesound.org/people/unfa/sounds/573745/, CC0)
* kuchtaa (https://freesound.org/people/kuchtaa/sounds/555418/, CC0)
* junkfood2121 (https://freesound.org/people/junkfood2121/sounds/242004/, CC0)
* KataVlogsYT (https://freesound.org/people/KataVlogsYT/sounds/324453/, CC0)
* peridactyloptrix (https://freesound.org/people/peridactyloptrix/sounds/202527/, CC0)
* dleigh (https://freesound.org/people/dleigh/sounds/346143/, CC0)
* YYZJJ (https://freesound.org/people/YYZJJ/sounds/508867/, CC0)
* Flash_Shumway (https://freesound.org/people/Flash_Shumway/sounds/113763/, CC0)
* shaundoogan (https://freesound.org/people/shaundoogan/sounds/465486/, CC0)
* Breviceps (https://freesound.org/people/Breviceps/sounds/445999/, CC0)

