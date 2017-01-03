@echo off
echo Desenvolvido por: Joao Orvalho e Joaquim Espada
echo Bem Vindo ao Instalador de Pacotes Automatico
IF EXIST redistributable_packages_x86.exe (
	redistributable_packages_x86.exe	
) ELSE (
	echo -----------------------------------------------
	echo Erro! Nao foi encontrado o instalador de pacotes. 
	echo Necessita de extrair o ficheiro ZIP! 
	pause
)
