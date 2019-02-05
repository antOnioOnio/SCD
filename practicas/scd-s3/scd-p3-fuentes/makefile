.SUFFIXES:
.PHONY:    pc,pc2,pcm,fp,f,fi,fc,clean

compilador := mpicxx -std=c++11
flagsc     := -g -Wall

pc: prodcons_exe
	mpirun -np  3 --oversubscribe ./$<

pc2: prodcons2_exe
	mpirun -np  3 --oversubscribe ./$<

pcm: prodcons2-mu_exe
	mpirun -np  10 --oversubscribe ./$<

fp: filosofos-sol_exe
	mpirun -np 10 --oversubscribe ./$<

fi: filosofos-interb_exe
	mpirun -np 10 --oversubscribe ./$<

f: filosofos_exe
	mpirun -np 10 --oversubscribe ./$<

fc: filosofos-cam_exe
	mpirun -np 11 --oversubscribe ./$<

fpos: filosofos-postre_exe
	mpirun -np 12 --oversubscribe ./$<

%_exe: %.cpp
	$(compilador) $(flagsc) -o $@ $<

clean:
	rm -rf *_exe *.dSYM
