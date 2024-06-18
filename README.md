My Pet project for learn programming with C++ using.
Additionally pratctice in GIT.

Goals:
- Learn C++ form Newbie to Middle developer (My first intordution in C++ in 1999)
- What happend with C++ in 11, 14, 17, 20. Move semantic, lambda function, constexpr - what is that ?? )
- Multithreading
- Network programming
- Library using practice (Boost for example)

About project.
Simple read linear equation matrix from text file and solve him by Gauss method.

Short history:
1) Develor matrix file (.mtx) generator - Done (https://github.com/Andreyspro/mtx-generator.git)
2) Develop application (Solver) for read and solving matrix files in one thread - Done.
3) Modify Solver for multithreading - Done.


What next:
- Develop network client server application: Solver-Client read file, send data to Solver-Server. Server calculate matrix and send answer back to Client;
- may be use SSE and another SIMD instructions.
- Port application to another device, may be Mikrotik router or ARM computer :)
- Refactoring for more performance
- Refactoring code for estetic view.