Sistemas Operativos, DEI/IST/ULisboa 2019-20
Grupo: 16
Autores: Guilherme Saraiva (93717); Sara Ferreira (93756)

2a entrega: Nesta pasta existem três executáveis, "tecnicofs-mutex", "tecnicofs-nosync", "tecnicofs-rwlock". No "lib"
            estão os ficheiros .c e .h auxiliares necessários para a compilação e execução do programa.

            Para executar faz-se
            ./tecnico-* inputFile outputFile NumberOfThreads NumberOfBuckets
            em que o * se subsitui por mutex, nosync ou rwlock dependendo do tipo de sincronia pretendido.

            Na diretoria está tembém o shell script runTests.sh que avalia o desempenho do TecnicoFS quando executado com
            diferentes argumentos e ficheiros de entrada.

            Para executar o shell script faz-se
            ./runTests.sh inputFilesDirectory outputFilesDirectory NumberOfThreads NumberOfBuckets




