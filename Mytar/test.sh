#! /bin/bash

if [ -x mytar ];
then
	clear
else
	echo "Error no existe el ejecutable"
	echo ""
	exit 1
fi
if [ -d tmp ];
then 
	echo "Carpeta tmp ya existe, borrar y crear otra"
	echo ""
	rm -r tmp
else
	echo "temp No EXiste, Creando"
	echo ""
fi
mkdir tmp
cd tmp
> file1.txt
echo "hola mundo" > file1.txt
> file2.txt
head -10 /etc/passwd > file2.txt
> file3.data
head -c 1024 /dev/urandom > file3.data
../mytar -c -f filetar.mtar file1.txt file2.txt file3.data
mkdir out
cd out
../../mytar -x -f ../filetar.mtar 
if diff file1.txt ../file1.txt && diff file2.txt ../file2.txt && diff file3.data ../file3.data
then 
    cd ../..
    echo "Correct"
    exit 0
else 
    cd ../..
    echo "Los ficheros no son iguales"
    exit 1    
fi


