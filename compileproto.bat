D:\nanopb-0.3.3-windows-x86\nanopb-0.3.3-windows-x86\generator-bin\protoc.exe -ocommunication communication.proto
c:\Python27\python.exe D:\nanopb-0.3.3-windows-x86\nanopb-0.3.3-windows-x86\generator\nanopb_generator.py communication 
del communication.c
move communication.pb.c communication.c
del communication.h
move communication.pb.h communication.h
sleep 5
