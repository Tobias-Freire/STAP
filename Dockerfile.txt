# Usa uma imagem base com gcc e ferramentas essenciais
FROM gcc:latest

# Define o diretório de trabalho dentro do container
WORKDIR /app

# Copia os arquivos do projeto para o container
COPY main.c Makefile ./

# Compila o programa
RUN make

# Define o número padrão de estudantes como argumento
ARG NUM_STUDENTS=5

# Executa o programa com o número de estudantes (pode ser sobrescrito com `--build-arg`)
CMD ["./main", "5"]