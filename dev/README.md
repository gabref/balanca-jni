# Development

## Folder Structure
* [com](./com/): Java code that calls the native library. There is a class that call the library, a class for testing Exception throws from JNI, and a class to test and run.
* [e1_balanca](./e1_balanca/): folder with the libraries for scale. Documentation [here](https://elgindevelopercommunity.github.io/group__g5.html)
* [src](./src/): folder with c code to create the DLL with JNI
* [release_dll](./release_dlls/): all compiled DLLs to use with Java application.
* [test-with-lib-directly](./test-with-lib-directly/): folder with test files, that use the scale DLL directly, without java. There is a C and python test.