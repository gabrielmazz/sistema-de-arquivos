#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include "module.h"
#include <unistd.h>

int main(void){
    //Declara o nome da partição
    char nome_da_particao[10];
    long long int numero_de_setores;

    //Declara o boot record
    boot_record br;
    root_directory rd;

    //Declara o arquivo
    FILE *arquivo;

    //Declara as variaveis
    char caminho[100];
    short int opcao;
    char nome[64];
    char comando[100];

    //Criar uma partição nova
    printf("Determine o nome da partição: ");
    scanf("%s", nome_da_particao);

    printf("Nome da partição: %s\n", nome_da_particao);
    strcpy(nome, nome_da_particao); strcat(nome, ".img");

    if(!arquivoExiste(nome))
    {
        printf("Determine o número de setores: ");
        scanf("%lld", &numero_de_setores);

        //Cria a partição
        sprintf(comando, "dd if=/dev/zero of=%s bs=512 count=%lld", nome, numero_de_setores);
        system(comando);
    }

    //Recebe o nome para o caminho
    strcpy(caminho, nome);

    //Abre o arquivo
    arquivo = fopen(caminho, "rb+");

    //Verifica se o arquivo foi aberto ou não
    if(arquivo == NULL){
        printf("Erro ao abrir o arquivo\n");
        return 1;
    }else
        printf("Arquivo aberto com sucesso\n");

    fseek(arquivo, 0, SEEK_END);  //Mover o cursor para o final do arquivo
    long tamanho = ftell(arquivo);  //Obter a posição atual do cursor, que é o tamanho do arquivo
    printf("Tamanho do arquivo: %ldMB\n\n", tamanho / 1024 / 1024);

    printf("Deseja formatar o arquivo?\n1 - Sim\t2 - Não\n");
    int op_form;
    scanf("%d", &op_form);

    if(op_form == 1){

        printf("Selecione uma opcao:\n");
        printf("1) Formata Arquivo\n");
        printf("2) Criar o Sistema de Arquivos - Default\n");
        printf("3) Criar o Sistema de Arquivos - Custom\n");
        printf("9) Sair do Sistema de Arquivos\n");
        printf("R: ");
        scanf("%hd", &opcao);

        switch(opcao){
            case 1:
                //Chama o formatador
                formatador_arq(arquivo, tamanho);
                break;
            case 2:
                printf("Criando o Sistema de Arquivos\n");
                br.bytes_por_setor = 512;
                br.setores_por_cluster = ceil((tamanho / br.bytes_por_setor) / pow(2, 32));
                br.setores_particulares = br.setores_por_cluster;
                br.numero_maximo_de_diretorios_raiz = (br.bytes_por_setor * br.setores_por_cluster) / 32;
                br.total_de_setores = tamanho / br.bytes_por_setor;
                br.cabeca = 1;
                break;
            
            case 3:
                printf("Defina qunatos bytes por setor você deseja: ");
                scanf("%hu", &br.bytes_por_setor);

                printf("Defina quantos setores por cluster você deseja: ");
                scanf("%hhu", &br.setores_por_cluster);

                printf("Defina quantos setores particulares você deseja: ");
                scanf("%hhu", &br.setores_particulares);

                printf("Defina o número máximo de diretórios raiz: ");
                scanf("%hu", &br.numero_maximo_de_diretorios_raiz);

                printf("Defina o total de setores: ");
                scanf("%u", &br.total_de_setores);

                printf("Defina a cabeça: ");
                scanf("%u", &br.cabeca);

            case 9:
                printf("Saindo do Sistema de Arquivos\n");
                break;

            default:
                printf("Opcao invalida.\n");
                break;
        }

        //Escreve o boot record:
        cria_boot_record(arquivo, &br);

        endereca_blocos(arquivo, &br);

    } else {
        fseek(arquivo, 0, SEEK_SET);
        fread((const char*)&br, sizeof(boot_record), 1, arquivo);
        //printf("caí no else\n");
    }
    
    while(op_form != 9){
        printf("Selecione uma opcao:\n");
        printf("1) Listar Arquivos\n");
        printf("2) Inserir Arquivo\n");
        printf("3) Remover Arquivo\n");
        printf("4) Copia do SA pro HD\n");
        printf("9) Sair do Sistema de Arquivos\n");
        printf("R: ");
        scanf("%d", &op_form);
        switch (op_form)
        {
        case 1:
            listaArquivos(arquivo, &br);
            break;
        case 2:
            insere_arquivo(arquivo, &br, rd);
            break;
        case 3:
            removerArquivos(arquivo, &br);
            break;
        case 4:
            copia_dentro_fora(arquivo, &br);
            break;
        case 9:
            printf("Obrigado pela preferência!\n");
            break;
        default:
            break;
        }
    }
    
    // Fecha o arquivo
    fclose(arquivo);
    
    return 0;
}