To compile project it is needed to change third line of CMake from
  set(dir_name test_dir)
to
  set(dir_name {curent directory name})

Also there might be different path to where the project is compiled to thus two last commands of CMake can throw errors.
Only thing you need to do is to find build's parent directory and put there data.json and ambient_sound.wav
