#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "module.h"
#include <math.h>

//Transforma valores em little endian
void escrever_int8_little_endian(FILE *arquivo, uint8_t valor){
    fwrite(&valor, sizeof(uint8_t), 1, arquivo);
}

void escrever_int16_little_endian(FILE *arquivo, uint16_t valor){
    unsigned char bytes[2];

    bytes[0] = valor & 0xFF;
    bytes[1] = (valor >> 8) & 0xFF;

    fwrite(bytes, sizeof(unsigned char), 2, arquivo);
}

void escrever_int24_little_endian(FILE *arquivo, uint32_t valor){
    unsigned char bytes[3];

    bytes[0] = valor & 0xFF;
    bytes[1] = (valor >> 8) & 0xFF;
    bytes[2] = (valor >> 16) & 0xFF;

    fwrite(bytes, sizeof(unsigned char), 3, arquivo);
}

void escrever_int32_little_endian(FILE *arquivo, uint32_t valor){
    unsigned char bytes[4];

    bytes[0] = valor & 0xFF;
    bytes[1] = (valor >> 8) & 0xFF;
    bytes[2] = (valor >> 16) & 0xFF;
    bytes[3] = (valor >> 24) & 0xFF;

    fwrite(bytes, sizeof(unsigned char), 4, arquivo);
}
//------------------------------------------------------------------

/*Formata o arquivo substituindo todo o arquivo por 0 indo byte a byte substituindo
  isso pode pesar na hora de executar o programa*/
void formatador_arq(FILE *arquivo, long tamanho){
    
    //Define o tempo de formatação
    time_t inicio = time(NULL);

    //Move o cursor para o primeiro byte do arquivo
    fseek(arquivo, 0, SEEK_SET);

    //Substitui cada byte por 0 formatando-o
    unsigned char zero = 0;
    for (long i = 0; i < tamanho; i++) {
        fwrite(&zero, sizeof(unsigned char), 1, arquivo);
    }

    printf("Arquivo formatado com sucesso\n");
    printf("Tempo para formatar: %ld segundos\n", time(NULL) - inicio);
}

void cria_boot_record(FILE *arquivo, boot_record* br){
    //Escreve o boot record:
    //Move o ponteiro para o começo do arquivo
    fseek(arquivo, 0, SEEK_SET);
    
    //Números de bytes por setor
    escrever_int16_little_endian(arquivo, br->bytes_por_setor);

    //Setores por cluster
    escrever_int8_little_endian(arquivo, br->setores_por_cluster);

    //Setores particulares
    escrever_int8_little_endian(arquivo, br->setores_particulares);

    //Número máximo de diretórios raiz
    escrever_int16_little_endian(arquivo, br->numero_maximo_de_diretorios_raiz);

    //Total de setores
    escrever_int32_little_endian(arquivo, br->total_de_setores);

    //Escreve cabeça
    escrever_int32_little_endian(arquivo, br->cabeca);
}

void insere_arquivo(FILE *arquivo, boot_record* br, root_directory rd){
    
    //Move o ponteiro para o primeiro byte do primeiro setor
    fseek(arquivo, (br->bytes_por_setor*br->setores_por_cluster), SEEK_SET);
    printf("Escrevendo o diretório raiz...\n");

    uint8_t byte;
    short int opcao;

    //Volta o ponteiro para o começo do primeiro setor
    fseek(arquivo, -1, SEEK_CUR);

    short int restante = conta_entradas_dir(arquivo, br);
    //printf("%hd\n", restante);
    //fseek(arquivo, -16, SEEK_CUR);

    //Quantas entradas vão ser digitadas
    printf("Quantas entradas voce quer dar?\n");
    unsigned short int teste;
    scanf("%hd", &teste);

    if (restante == 0) {
        printf("Não há entradas disponíveis");
    } else {
    //Impede de escrever mais que a qtd maxima de entradas no root dir
        while(teste > restante)
        {
            printf("Impossível fazer tantos pedidos, limite alcançado.\nRepense quantas entradas voce quer.\n");
            scanf("%hd", &teste);
        }

        //Escreve o diretório raiz
        for(size_t i=0; i < teste; i++){
            
            fseek(arquivo, 1, SEEK_CUR);
            byte = fgetc(arquivo);
            fseek(arquivo, -1, SEEK_CUR);
            //printf("ponteiro antes %ld\n", ftell(arquivo));
            
            if((byte == 0) || (byte == 229)){
                printf("%ld Arquivo\n", i);
                printf("Selecione se quer diretório ou arquivo\n");
                printf("1 - Arquivo\n");
                printf("2 - Diretório\n");
                printf("3 - Copia arquivo do disco para o sistema\n");
                printf("R: ");
                scanf("%hd", &opcao);

                switch(opcao){
                    case 1:
                        rd = escreve_variaveis_arquivo(rd);
                        escreve_1_arquivo(arquivo, br, rd);
                        break;

                    case 2:
                        rd = escreve_variaveis_diretorio(rd);
                        escreve_1_arquivo(arquivo, br, rd);
                        break;
                    case 3:
                        rd = escreve_variaveis_arquivo(rd);
                        printf("passei da declaração de valor\n");
                        copia_fora_dentro(arquivo, br, rd);
                        break;
                }
                
                fseek(arquivo, -1, SEEK_CUR);
            } else {
                fseek(arquivo, 31, SEEK_CUR);
                i--;
            }

            //printf("ponteiro depois %ld\n", ftell(arquivo));
        }
    }
}

void endereca_blocos(FILE *arquivo, boot_record* br){
    //Declara o ponteiro para a área de cluster
    int pont2 = (br->bytes_por_setor * br->setores_por_cluster) + (br->bytes_por_setor * br->setores_particulares);
    printf("pont2: %d\n", pont2);
        
    //declarar ponteiros para próximo cluster
    fseek(arquivo, pont2, SEEK_SET);
    //O -3 por causa que subtrai o br, rd e ultimo cluster 
    for(int i = 1; i < (br->total_de_setores/br->setores_por_cluster) - 2; i++)
    {
        int apontar = i + 1;
        fwrite(&apontar, sizeof(int), 1, arquivo);
        fseek(arquivo, 508, SEEK_CUR);
    }
    int invalido = -1;
    fwrite(&invalido, sizeof(int), 1, arquivo);
}

void zera_diretorio(FILE *arquivo){
    unsigned char zero = 0;
    for (long i = 0; i < 32; i++)
        fwrite(&zero, sizeof(unsigned char), 1, arquivo);
}

void escreve_1_arquivo(FILE *arquivo, boot_record* br, root_directory rd){
    unsigned char zero = 0;
    rd.parte_zerada = 0;
    unsigned int cluster;

    //Escreve o diretório raiz
    //Escreve o nome do arquivo
    for (int j = 0; j < 15; j++) {
        if (j < strlen(rd.nome))
            fwrite(&rd.nome[j], sizeof(unsigned char), 1, arquivo);
        else
            fwrite(&zero, sizeof(unsigned char), 1, arquivo);
    }

    //Escreve a extensão do arquivo
    for (int j = 0; j < 5; j++) {
        if (j < strlen(rd.extensao))
            fwrite(&rd.extensao[j], sizeof(unsigned char), 1, arquivo);
        else
            fwrite(&zero, sizeof(unsigned char), 1, arquivo);
    }

    //Escreve os atributos do arquivo
    fwrite(&rd.atributos, sizeof(unsigned char), 1, arquivo);

    fseek(arquivo, 4, SEEK_CUR);

    //Escreve o tamanho do arquivo
    escrever_int32_little_endian(arquivo, rd.tamanho_do_arquivo);

    fseek(arquivo, -8, SEEK_CUR);
    long temp = ftell(arquivo);
    
    cluster = encontra_clusteres_livres(arquivo, br, rd.tamanho_do_arquivo);
    fseek(arquivo, temp, SEEK_SET);
    escrever_int32_little_endian(arquivo, cluster);


    fseek(arquivo, 3, SEEK_CUR);
    //Joga o ponteiro para frente por causa da area zerada
    escrever_int32_little_endian(arquivo, rd.parte_zerada);

}

root_directory escreve_variaveis_arquivo(root_directory rd){

    //Escreve o nome do arquivo
    printf("Digite o nome do arquivo: ");
    scanf("%s", rd.nome);

    //Escreve a extensão do arquivo
    printf("Digite a extensão do arquivo: ");
    scanf("%s", rd.extensao);

    //Escreve os atributos do arquivo
    rd.atributos = 0x14;
    //printf("Digite os atributos do arquivo: ");
    //scanf("%hhx", &rd.atributos);

    //Escreve o tamanho do arquivo
    //printf("Digite o tamanho do arquivo: ");
    //scanf("%u", &rd.tamanho_do_arquivo);
  
    return rd;
}

root_directory escreve_variaveis_diretorio(root_directory rd){

    //Escreve o nome do arquivo
    printf("Digite o nome do diretorio: ");
    scanf("%s", rd.nome);

    //Escreve a extensão do arquivo
    strcpy(rd.extensao, "");

    //Escreve os atributos do arquivo
    rd.atributos = 0x0A;

    //Escreve o tamanho do arquivo
    rd.tamanho_do_arquivo = 1;
  
    return rd;
}
//------------------------------------------------------------------

void listaArquivos(FILE *arquivo, boot_record* br){
    char nome_listar[15];
    char extensao_listar[5];
    char listar[21];

    memset(&listar, 0, sizeof(char)*21);
    memset(&nome_listar, 0, sizeof(char)*15);
    memset(&extensao_listar, 0, sizeof(char)*5);
    
    printf("Listando os arquivos...\n");
    fseek(arquivo, (br->bytes_por_setor*br->setores_por_cluster), SEEK_SET);
    for (int i = 0; i < br->numero_maximo_de_diretorios_raiz; i++)
    {
        int status = fgetc(arquivo);
        if ((status != 0) && (status != 229))
        {
            fseek(arquivo, -1, SEEK_CUR);
            for(int j=0;j<15;j++)
            {
                if (fgetc(arquivo)!=0)
                {
                    fseek(arquivo, -1, SEEK_CUR);
                    nome_listar[j] = fgetc(arquivo);
                }
                else
                    continue;
            }

            for(int j=0;j<5;j++)
            {
                if (fgetc(arquivo)!=0)
                {
                    fseek(arquivo, -1, SEEK_CUR);
                    extensao_listar[j] = fgetc(arquivo);
                }
                else
                    continue;
            }
            strcpy(listar, nome_listar); strcat(listar, "."); strcat(listar, extensao_listar);
            printf("%d) Arquivo: %s\n", i, listar);
            memset(&listar, 0, sizeof(char)*21);
            memset(&nome_listar, 0, sizeof(char)*15);
            memset(&extensao_listar, 0, sizeof(char)*5);
            fseek(arquivo, 12, SEEK_CUR);
        }
        else
            fseek(arquivo, 31, SEEK_CUR);
    }
}

void removerArquivos(FILE *arquivo, boot_record* br){
    int op_rmv;
    unsigned char excluido = 0xE5;
    listaArquivos(arquivo, br);
    printf("Qual o número do arquivo para remover? ");
    scanf("%d", &op_rmv);
    fseek(arquivo, (br->bytes_por_setor * br->setores_por_cluster)+(op_rmv*32), SEEK_SET);
    fwrite(&excluido, sizeof(unsigned char), 1, arquivo);

    uint32_t tamanho, primeiro_cluster;
    fseek(arquivo, 20, SEEK_CUR);
    fread(&primeiro_cluster, sizeof(uint32_t), 1, arquivo);
    fread(&tamanho, sizeof(uint32_t), 1, arquivo);

    rearranja_clusteres(arquivo, br, tamanho, primeiro_cluster);

    printf("Arquivo removido!\n");
}

int arquivoExiste(const char *caminho) {
    if (access(caminho, F_OK) != -1) {
        // O arquivo existe
        return 1;
    } else {
        // O arquivo não existe
        return 0;
    }
}

unsigned short int conta_entradas_dir(FILE *arquivo, boot_record* br){
    unsigned short int restante = br->numero_maximo_de_diretorios_raiz;
    int status;
    fseek(arquivo, 1, SEEK_CUR);
    long pont_aux = ftell(arquivo);
    //printf("%ld\n", pont_aux);

    //printfs nessa função são puramente pra debug
    //printf("restante começou como %hd\n", restante);

    for (size_t i = 0; i < br->numero_maximo_de_diretorios_raiz; i++)
    {
        status = fgetc(arquivo);
        if (status == 229 || status == 0){
            fseek(arquivo, 31, SEEK_CUR);
        } else {
            restante--;
            fseek(arquivo, 31, SEEK_CUR);
        }
        //printf("status é %d\n", status);
        //printf("restante é %hd\n", restante);
    }
    
    //printf("restante é %hd\n", restante);
    fseek(arquivo, (pont_aux-1), SEEK_SET);
    
    return restante;
}

//--------------------------------------------------------------------

//Acha os clusteres em que vai armazenar um arquivo
unsigned int encontra_clusteres_livres(FILE *arquivo, boot_record *br, int tamanho){

    //gera a equação em double, pra que possamos fazer um ceil, para termos a quantidade adequada de clusteres para serem ocupados
    double equacao = ((double)tamanho/((double)br->bytes_por_setor*(double)br->setores_por_cluster));
    //printf("valor da equacao = %lf\n", equacao);
    uint32_t qtd_clusteres = ceil(equacao);
    //printf("qtd de clusteres necessarios %d\n", qtd_clusteres);

    //Salva valor atual da cabeça da lista
    uint32_t cabeca = br->cabeca;
    int tam_cluster = (br->bytes_por_setor*br->setores_por_cluster);
    int ini_area_dados = tam_cluster*2;
    uint32_t base, ant, cluster_inicial;

    //Encontra onde está a cabeça
    fseek(arquivo, (ini_area_dados)+(tam_cluster*(cabeca - 1)), SEEK_SET);
    ant = cabeca;
    cluster_inicial = cabeca;
    //printf("%u\n", cluster_inicial);

    //Percorre o laço pra ver se acha i clusteres contíguos na memória
    for(size_t i = 0; i < qtd_clusteres; i++) {
        fread(&base, sizeof(int), 1, arquivo);
        if ((base - ant) == 1){
            ant = base;
            fseek(arquivo, 508, SEEK_CUR);
        } else if ((base - ant) > 1) {
            i = 0;
            ant = base;
            cluster_inicial = base;
            fseek(arquivo, (ini_area_dados + (tam_cluster * (base - 1))), SEEK_SET);
        } else if (base == 0xFFFFFFFF && i < qtd_clusteres) {
            printf("Não é possível armazenar o arquivo.\nAbortando.\n");
            return EXIT_SUCCESS;
        }
        
    }

    //Definimos variável de cluster final porque precisamos de uma variável para escrever na memória
    uint32_t cluster_final = cluster_inicial + qtd_clusteres;
    //printf("cluster inicial é %u\ncabeca = %u\n", cluster_inicial, br->cabeca);

    if(cluster_inicial == br->cabeca){
        fseek(arquivo, 10, SEEK_SET);
        fwrite(&cluster_final, sizeof(uint32_t), 1, arquivo);
        //printf("entrei no if\n");
        br->cabeca = cluster_final;
    }

    printf("%u\n", br->cabeca);
    
    return cluster_inicial;

}

//Função que rearranja clusteres livres quando um arquivo é removido
void rearranja_clusteres(FILE *arquivo, boot_record *br, uint32_t tamanho, uint32_t primeiro_cluster) {
    double equacao = ((double)tamanho/((double)br->bytes_por_setor*(double)br->setores_por_cluster));
    uint32_t qtd_clusteres = ceil(equacao);

    //uint32_t ultimo_cluster = primeiro_cluster + qtd_clusteres;

    uint32_t cabeca;

    int tam_cluster = br->bytes_por_setor*br->setores_por_cluster;

    unsigned char zero = 0;

    fseek(arquivo, 10, SEEK_SET);
    fread(&cabeca, sizeof(uint32_t), 1, arquivo);
    //printf("%u é o valor da cabeça\n%u é o valor do primeiro cluster\n", cabeca, primeiro_cluster);
    if (primeiro_cluster < cabeca) {
        fseek(arquivo, ( (tam_cluster * 2) + (tam_cluster * (primeiro_cluster - 1) ) ), SEEK_SET);

        uint32_t cluster_relativo = primeiro_cluster;
        for (uint32_t i = 0; i < qtd_clusteres - 1; i++){
            cluster_relativo++; 

            //printf("%u é o cluster relativo\n%u é o iterador i\n", cluster_relativo, i);
            fwrite(&cluster_relativo, sizeof(uint32_t), 1, arquivo);
            for(int j = 0; j < 12; j++)
                fwrite(&zero, sizeof(unsigned char), 1, arquivo);
            fseek(arquivo, (tam_cluster - 16), SEEK_CUR);
        }
        
        fwrite(&cabeca, sizeof(uint32_t), 1, arquivo);
        for(int j = 0; j < 12; j++)
            fwrite(&zero, sizeof(unsigned char), 1, arquivo);

        //printf("a cabeça é %u", cabeca);
        cabeca = primeiro_cluster;
        //printf("cabeça passou a ser %u", cabeca);

        fseek(arquivo, 10, SEEK_SET);
        fwrite(&cabeca, sizeof(uint32_t), 1, arquivo);

        br->cabeca = cabeca;
    } else {
        fseek(arquivo, ((tam_cluster*2) + (tam_cluster * cabeca - 1)), SEEK_SET);
        uint32_t cluster_ponteiro;
        uint32_t aux;

        fread(&cluster_ponteiro, sizeof(uint32_t), 1, arquivo);
        while (cluster_ponteiro < primeiro_cluster){
            aux = cluster_ponteiro;
            fseek(arquivo, (tam_cluster - 4), SEEK_CUR);
            fread(&cluster_ponteiro, sizeof(uint32_t), 1, arquivo);
        }

        fseek(arquivo, ((tam_cluster*2) + (tam_cluster * aux - 1)), SEEK_SET);
        fwrite(&primeiro_cluster, sizeof(uint32_t), 1, arquivo);

        fseek(arquivo, ( (tam_cluster * 2) + (tam_cluster * (primeiro_cluster - 1) ) ), SEEK_SET);

        uint32_t cluster_relativo = primeiro_cluster;
        for (uint32_t i = 0; i < qtd_clusteres - 1; i++){
            cluster_relativo++; 

            printf("%u é o cluster relativo\n%u é o iterador i\n", cluster_relativo, i);
            fwrite(&cluster_relativo, sizeof(uint32_t), 1, arquivo);
            for(int j = 0; j < 12; j++)
                fwrite(&zero, sizeof(unsigned char), 1, arquivo);
            fseek(arquivo, (tam_cluster - 16), SEEK_CUR);
        }

        fwrite(&aux, sizeof(uint32_t), 1, arquivo);
        for(int j = 0; j < 12; j++)
            fwrite(&zero, sizeof(unsigned char), 1, arquivo);
    }
}

//Copiar de fora para dentro (HD -> SA)
void copia_fora_dentro(FILE *arquivo, boot_record *br, root_directory rd){
    unsigned char zero = 0;
    rd.parte_zerada = 0;
    unsigned int cluster;
    char caminho[512] = "";
    
    printf("Digite o caminho até o arquivo, mas sem incluí-lo (você já passou ele antes)\n");
    //fgets(caminho, 100, stdin);
    //strcpy(caminho, "D:\\Faculdade\\3º Ano\\SO\\S Arquivos\\so");
    //strcat(caminho, "\\");
    strcat(caminho, rd.nome);
    strcat(caminho, ".");
    strcat(caminho, rd.extensao);

    printf("%s\n", caminho);

    FILE *pont_arq_ext;
    printf("%s\n", caminho);
    pont_arq_ext = fopen(caminho, "rb+");

    //Escreve o diretório raiz
    //Escreve o nome do arquivo
    for (int j = 0; j < 15; j++) {
        if (j < strlen(rd.nome))
            fwrite(&rd.nome[j], sizeof(unsigned char), 1, arquivo);
        else
            fwrite(&zero, sizeof(unsigned char), 1, arquivo);
    }

    //Escreve a extensão do arquivo
    for (int j = 0; j < 5; j++) {
        if (j < strlen(rd.extensao))
            fwrite(&rd.extensao[j], sizeof(unsigned char), 1, arquivo);
        else
            fwrite(&zero, sizeof(unsigned char), 1, arquivo);
    }

    //Escreve os atributos do arquivo
    fwrite(&rd.atributos, sizeof(unsigned char), 1, arquivo);

    fseek(arquivo, 4, SEEK_CUR);

    //Escreve o tamanho do arquivo
    fseek(pont_arq_ext, 0, SEEK_END);
    rd.tamanho_do_arquivo = ftell(pont_arq_ext);
    escrever_int32_little_endian(arquivo, rd.tamanho_do_arquivo);
    

    fseek(arquivo, -8, SEEK_CUR);
    long temp = ftell(arquivo);
    
    cluster = encontra_clusteres_livres(arquivo, br, rd.tamanho_do_arquivo);
    fseek(arquivo, temp, SEEK_SET);
    escrever_int32_little_endian(arquivo, cluster);


    fseek(arquivo, 3, SEEK_CUR);
    //Joga o ponteiro para frente por causa da area zerada
    escrever_int32_little_endian(arquivo, rd.parte_zerada);

    int tam_cluster = br->setores_por_cluster * br->bytes_por_setor;

    fseek(arquivo, (tam_cluster*2) + ((cluster-1) * tam_cluster), SEEK_SET);
    fseek(pont_arq_ext, 0, SEEK_SET);

    for (uint32_t i = 0; i < rd.tamanho_do_arquivo; i++){
        char byte;
        fread(&byte, sizeof(char), 1, pont_arq_ext);
        fwrite(&byte, sizeof(char), 1, arquivo);
    }
}

void copia_dentro_fora(FILE *arquivo, boot_record *br){
    int op_cpy;
    int tam_cluster = br->setores_por_cluster * br->bytes_por_setor;
    
    listaArquivos(arquivo, br);
    printf("Qual o número do arquivo para copiar? ");
    scanf("%d", &op_cpy);
    fseek(arquivo, (tam_cluster)+(op_cpy*32), SEEK_SET);

    char extensao_listar[5];
    char nome_listar[15];
    char nome[25];
    //fseek(arquivo, -1, SEEK_CUR);
    for(int j=0;j<15;j++)
    {
        if (fgetc(arquivo)!=0)
        {
            fseek(arquivo, -1, SEEK_CUR);
            nome_listar[j] = fgetc(arquivo);
            //printf("%s\n", nome_listar);
        }
        else
            continue;
    }

    for(int j=0;j<5;j++)
    {
        if (fgetc(arquivo)!=0)
        {
            fseek(arquivo, -1, SEEK_CUR);
            extensao_listar[j] = fgetc(arquivo);
        }
        else
            continue;
    }

    uint32_t tamanho, primeiro_cluster;
    fseek(arquivo, 2, SEEK_CUR);
    fread(&primeiro_cluster, sizeof(uint32_t), 1, arquivo);
    fread(&tamanho, sizeof(uint32_t), 1, arquivo);

    int i = 0;

    FILE *pont_arq;

    strcpy(nome, nome_listar); strcat(nome, "."); strcat(nome, extensao_listar);
    
    pont_arq = fopen(nome, "rb+");

    fseek(arquivo, (tam_cluster*2) + ((primeiro_cluster) * tam_cluster), SEEK_SET);
    for (uint32_t j = 0; j < tamanho; j++){
        char byte;
        fread(&byte, sizeof(char), 1, arquivo);
        fwrite(&byte, sizeof(char), 1, pont_arq);
    }

    printf("Cópia finalizada com sucesso\n\n");

}


