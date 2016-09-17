g++ -O0 -g3 -Wall -c "pack-capture/capture/v4lcap.cpp" -o "pack-capture/capture/v4lcap.o"
ar -r "pack-capture/libpack-capture.a" "pack-capture/capture/v4lcap.o"
g++ -I"pack-capture" `pkg-config gtkmm-3.0 --cflags` -O3 -Wall -c "pack-capture-gui/capture-gui/V4LInterface-aux.cpp" -o "pack-capture-gui/capture-gui/V4LInterface-aux.o"
g++ -I"pack-capture" `pkg-config gtkmm-3.0 --cflags` -O3 -Wall -c "pack-capture-gui/capture-gui/V4LInterface-events.cpp" -o "pack-capture-gui/capture-gui/V4LInterface-events.o"
ar -r  "pack-capture-gui/libpack-capture-gui.a"  "pack-capture-gui/capture-gui/V4LInterface-aux.o" "pack-capture-gui/capture-gui/V4LInterface-events.o"

g++ -I"pack-capture-gui" -I"pack-capture" -O0 -g3 -Wall -c "cc/main.cpp" -o "cc/main.o" `pkg-config gtkmm-3.0 --cflags`
g++ -L"pack-capture" -L"pack-capture-gui" -L"/usr/local/lib" -L"/lib64" -o "cc/CamCal"  "cc/main.o" -lpack-capture-gui -lpack-capture -lopencv_shape -lopencv_stitching -lopencv_objdetect -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core `pkg-config gtkmm-3.0 libv4l2 libv4lconvert --libs`
./cc/CamCal