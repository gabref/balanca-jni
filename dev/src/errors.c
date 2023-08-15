#include "errors.h"

char * intToStr(int intToConvert, char *strConverted) {
    sprintf(strConverted, "%d", intToConvert);
}

void mapE1ErrorCodesToSara(E1ErrorCode errorCode, char *saraError) {
    // initialize the saraError Buffer to empty string
    memset(saraError, 0, sizeof(saraError));

    switch(errorCode) {
        case ERRO_CARGA_DLL:
            intToStr(SARA_ERRO_CARGA_DLL_FABRICANTE, saraError);
            break;

        case TIPO_INVALIDO:
        case MODELO_INVALIDO:
        case DISPOSITIVO_NAO_EXISTE:
        case PERMISSAO_NEGADA:
        case SERIAL_TIMEOUT:
        case DISPOSITIVO_REMOVIDO_INESPERADAMENTE:
            intToStr(SARA_PERIFERICO_DESLIGADO, saraError);
            break;

        case PORTA_FECHADA:
        case CONEXAO_NEGADA:
        case CONEXAO_ATIVA:
        case BAUDRATE_INVALIDO:
        case ERRO_SERIAL_DESCONHECIDO:
        case BAL_PROTOCOLO_INVALIDO:
        case BAL_PROTOCOLO_EM_USO:
        case BAL_PROTOCOLO_NAO_SUPORTADO:
        case BAL_PROTOCOLO_NAO_SUPORTADO_PELAS_CONFIGS_SERIAIS:
        case BAL_NENHUM_PROTOCOLO_EM_USO:
        case BAL_BAUDRATE_NAO_SUPORTADO:
        case BAL_LENGTH_NAO_SUPORTADO:
        case BAL_PARITY_NAO_SUPORTADO:
        case BAL_STOPBITS_NAO_SUPORTADO:
        case BAL_BAUDRATE_INVALIDO:
        case BAL_LENGTH_INVALIDO:
        case BAL_PARITY_INVALIDO:
        case BAL_STOPBITS_INVALIDO:
        case BAL_COMBINACAO_DE_PARAMETROS_INVALIDA:
        case BAL_CONFIGS_SERIAIS_NAO_SUPORTADAS_PELO_PROTOCOLO:
            intToStr(SARA_ERRO_ABERTURA_OU_FECHAMENTO_PORTA, saraError);
            break;

        case BAL_QTD_LEITURAS_INVALIDA:
        case BAL_PRECO_INVALIDO:
        case BAL_FALHA_NA_LEITURA_DO_PESO:
        case BAL_FALHA_AO_ENVIAR_PRECO:
            intToStr(SARA_TIMEOUT, saraError);
            break;

        case ERRO_NUMERO_SERIE:
        case ERRO_OBTENCAO_NUMERO_SERIE:
            intToStr(SARA_ERRO_OBTER_NUMERO_SERIE, saraError);
            break;

        case DISPOSITIVO_JA_ESTA_ABERTO:
        case RECURSO_INDISPONIVEL:
        case OPERACAO_NAO_SUPORTADA:
        case BAL_CONEXAO_ATIVA:
        case BAL_BALANCA_EM_USO:
        case BAL_BALANCA_INVALIDA:
        case BAL_NENHUMA_BALANCA_EM_USO:
        case CHAMADA_NAO_PERMITIDA:
        case BAL_COMANDO_NAO_SUPORTADO_PELA_BALANCA:
        case BAL_COMANDO_NAO_SUPORTADO_PELO_PROTOCOLO:
            intToStr(SARA_EXCECAO_GENERICA, saraError);
            break;

        default:
            intToStr(SARA_EXCECAO_GENERICA, saraError);
    }
}