set -e
clang++ -std=c++11 -O2 -s -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections -Wno-c++11-narrowing -Wl,--gc-sections -Wno-invalid-source-encoding -DNDEBUG -D_POSIX_THREADS -D_POSIX_READER_WRITER_LOCKS -D__LINUX__=1 main.cpp ../win/render.cpp -I../../ -o../../../bin/OpenResident -lX11 -lGL -lm -lpthread -lpulse-simple -lpulse
strip ../../../bin/OpenResident --strip-all --remove-section=.comment --remove-section=.note
