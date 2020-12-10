Sistemas Operativos, DEI/IST/ULisboa 2019-20
Grupo: 16
Autores: Guilherme Saraiva (93717); Sara Ferreira (93756)
1a entrega: Esta entrega consiste na leitura de comandos proveniente de um ficheiro de entrada, a execucao desses comandos e' feita de 3 maneiras diferentes:
			uma sem sincronizacao e outras duas com sincronizacao (mutex / rwlock).
			O resultado final dos comandos e' extraido para um ficheiro de saida.

Adicoes: Na nossa resolucao apenas alteramos o ficheiro main.c onde implementamos novas funcoes:
			- void checkNumberThreads();
			- void setThreadPool();
			- void lock(char[], int);
			- void unlock(int n);
			- void lockError(int).
