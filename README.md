### Description
----
The package is a mouse cursor control application that detects the position of the hand on surface and displays it on the screen, using [Leap Stereo IR](https://www.ultraleap.com/product/stereo-ir-170/). Essentially, the application matches a user defined surface and PC display. The application also recognizes the gesture of pressing on the surface. Collecting data from the Leap Stereo IR device allows determine the position of the hands in 3D space. The package consists of two parts:
1)	calibration_app.exe – console app, helping with calibration process;
2)	tracking_app.exe – main app for control the cursor. Could perform cursor movement, click, double click and right click.
      The detailed description you can read below.
#### Supported gestures
-----
1)	left click – the index finger tap;
2)	double click – the index finger double tap;
3)	right click – both index and middle finger tap;
4)	cursor movement – just moving index finger.
      You can customize following settings:
#### Calibration app
To calibrate, you need:
1) select any surface;
2) place the Leap Stereo near the surface surface (better accuracy is performed, when sensor position is in front of surface);
3) walk through the vertices of the target surface with your index finger (an area surface should be a rectangle);
4) obtain the calib.txt file for calibration and then use it for tracking app.
#### Control app
The app is proposed to replace mouse and control pc within fingers of one hand.
The app have the external console and after start could manage projector/monitor as regular mouse. After the calibration process you need to copy calib.txt into the current app directory to give an app current calibration setup. Also you can modify the following standart settings in config.txt:
1)	interaction_threshold – distance in mm from hand to screen, when gestures begin to interact with screen. By default is equal to 100 mm.
2)	press_threshold  - distance in mm from hand to screen, when the click starts working. By default is equal to 10 mm;
3)	interdigital_threshold – the distance between index and middle finger, when when the fingers are joined together (for right click gecture).
4)	max_doubleclick_delay – max delay between clicks to call double click. By default is 500 ms.
#### Requirements
----
1. Install OpenCV 4+;
2. Install [LeapSDK](https://developer.leapmotion.com/tracking-software-download);
3. Copy [spdlog](https://github.com/gabime/spdlog) header-only part into root_directory/3rdparty/.
   Using cmake, you need to mannually set path to opencv and leapsdk.
#### Notes
----
Software, which Ultraleap provides with their devices:
1) XR integration bindings (solutions for VR);
2) TouchFree high-level app;
3) LeapC - low-level c-library.

TouchFree is the app for displays, with calibration interface. It could be used with ultraleap contollers for cursor-control on Windows. According to the idea of developers, it might replace mouse and you will be able to move cursor across the screen with ypur hands.
While trying it, I found out the main problem - TouchFree support only one single click gesture. Additional problem is that cursor
is not properly detecting, causing cursor fluctuations with the screen.
You can check it [here](https://developer.leapmotion.com/?_gl=1*awh1qy*_ga*MTc2ODQ1MzA2MS4xNjgyMzM2MDUx*_ga_5G8B19JLWG*MTY4Mjg1MTQ2OC4xMy4xLjE2ODI4NTI4MDEuNTguMC4w#)

LeapC is the low-level C-api, which provides access to the device.
Functions:
1) obtain images from 2 cameras of device;
2) get frame rate;
3) get 3D position of each joint of the detected hands.
   Unfortunately, library is being developing and Ultraleap frequently changes the API architecture and functions.
   2.x - 3.x provide proper docs, C++, python 2.7 and other languages APIs. Clear usage, but some function are defined depricated
   in later versions, due to architecture mistakes.
   4.x - still provide good docs, but I didn't find sources.
   5.x - provide small guide and docs on Ultraleap website with couple of https://docs.ultraleap.com/tracking-api/ examples.
   Using the api, I found out that device info don't provide maximum capture range as well as horizontal and vertical fov of device.
   Making experiments, I have set the maximum acceptable distance for hand recognition 70cm.