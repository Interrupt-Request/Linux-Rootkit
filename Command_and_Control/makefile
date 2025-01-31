
#Builds a project and all of its dependencies. el fichero ejecutable se encuentra en /bin/debug/net8 el que no tiene extension
build:
	dotnet build
#Ejecuta el programa
run:
	dotnet run
clean:
	dotnet clean
	rm -r -f ./bin
	rm -r -f ./obj
	rm -r -f ./output
	
#Publish" does a build (with some extra options, such as platform/architecture) , and then runs some extra publishing steps. 
publish:
	dotnet publish -r linux-x64 --self-contained false 

all: publishContained copy

#https://learn.microsoft.com/en-us/dotnet/core/deploying/single-file/overview?tabs=cli
#https://www.variablenotfound.com/2020/02/publicacion-self-contained-y-single.html
publishContained:
	dotnet publish -r linux-x64 --self-contained true -c release -o output

copy:
	mkdir -p ../build
#	cp  ./bin/Release/net8.0/linux-x64/publish/Command_and_Control ../build
	cp -f ./output/Command_and_Control ../build
	