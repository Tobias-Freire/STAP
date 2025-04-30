# STAP
Sleeping Teaching Assistant Problem

## Sobre
Um assistente de ensino (TA - Teaching Assistant) ajuda alunos em um laboratório de programação, mas dorme quando não há alunos precisando de ajuda. O laboratório tem um número limitado de cadeiras de espera, e os alunos que precisam de ajuda podem ter que esperar se o TA estiver ocupado.

### Situações possíveis

- Se um aluno chega e o TA está dormindo, ele acorda o TA e recebe ajuda imediatamente.

- Se o TA está ocupado ajudando outro aluno, o aluno que chega pode esperar em uma das cadeiras disponíveis.

- Se todas as cadeiras estiverem ocupadas, o aluno vai embora sem receber ajuda.

- O TA atende os alunos um por um, e volta a dormir quando não há mais alunos esperando.


## Como rodar
> Pré-requisitos: Compilador gcc e Docker instalados devidamente

### Com Makefile
- Rode o comando `make`para compilar o programa

- Rode o comando `./main`para rodar o programa. Você pode passar o número de estudantes como argumento, exemplo: `./main 3`

- Para limpar os arquivos gerados no build, rode `make clean`

### Com Docker
- Rode o comando `sudo docker build -t stap .` para fazer o build e gerar a imagem do container

- Rode o comando `sudo docker run --rm stap` para rodar o container. Você pode passar o número de estudantes como argumento, exemplo: `sudo docker run --rm stap ./main 3`

>**Nota**: Ao rodar com Makefile, os prints no terminal ocorrem de forma dinâmica, de modo que se pode ver o que está acontecendo em tempo real. Ao rodar com Docker, os prints somente são mostrados no terminal ao final da execução.