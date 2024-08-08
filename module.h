typedef struct{
    uint16_t bytes_por_setor;
    uint8_t setores_por_cluster;
    uint8_t setores_particulares;
    uint16_t numero_maximo_de_diretorios_raiz;
    uint32_t total_de_setores;
    uint32_t cabeca;
}__attribute__((packed)) boot_record;

typedef struct{
    uint8_t status;
    char nome[15];
    char extensao[5];
    uint8_t atributos;
    uint32_t primeiro_cluster;
    uint32_t tamanho_do_arquivo;
    uint32_t parte_zerada;
} root_directory;

void escrever_int8_little_endian(FILE *arquivo, uint8_t valor);
void escrever_int16_little_endian(FILE *arquivo, uint16_t valor);
void escrever_int24_little_endian(FILE *arquivo, uint32_t valor);
void escrever_int32_little_endian(FILE *arquivo, uint32_t valor);

void formatador_arq(FILE *arquivo, long tamanho);
void cria_boot_record(FILE *arquivo, boot_record* br);
void endereca_blocos(FILE *arquivo, boot_record* br);
void insere_arquivo(FILE *arquivo, boot_record* br, root_directory rd);
void zera_diretorio(FILE *arquivo);
void escreve_1_arquivo(FILE *arquivo, boot_record* br, root_directory rd);


root_directory escreve_variaveis_arquivo(root_directory rd);
root_directory escreve_variaveis_diretorio(root_directory rd);


void listaArquivos(FILE *arquivo, boot_record* br);
void removerArquivos(FILE *arquivo, boot_record* br);
int arquivoExiste(const char *caminho);
unsigned short int conta_entradas_dir(FILE *arquivo, boot_record* br);


unsigned int encontra_clusteres_livres(FILE *arquivo, boot_record *br, int tamanho);
void rearranja_clusteres(FILE *arquivo, boot_record *br, uint32_t tamanho, uint32_t primeiro_cluster);
void copia_fora_dentro(FILE *arquivo, boot_record *br, root_directory rd);
void copia_dentro_fora(FILE *arquivo, boot_record *br);

