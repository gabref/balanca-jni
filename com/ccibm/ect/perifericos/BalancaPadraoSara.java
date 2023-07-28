package com.ccibm.ect.perifericos;

public class BalancaPadraoSara {

    static {
        try {
            System.loadLibrary("ECTSARA_BAL_ELGIN_DP30CK");
        } catch(UnsatisfiedLinkError e) {
            System.out.println("Problems finding library: " + e.getMessage());
        }
    }

    /**
     * Obtém o número de série do equipamento
     * 
     * @return valor do número de série armazenado na memória interna do equipamento
     * @exception java.lang.Exception
     */
    private native String obterNumeroSerie() throws Exception;

    /**
     * Obtém o peso de um dado object
     * 
     * @param portaSerial valor padrão da porta de entrada, se não informado o valor utilizado pela DLL
para comunicação deverá ser 3 – COM3 – se informado, os valores referenciados 
são os mesmos do atributo portaSerial da seção subsequente
     * @return Peso lido em gramas, retornar -1 quando o peso não estiver estável
     * @exception java.lang.Exception
     */
    private native String lerPeso(int portaSerial) throws Exception;

    /**
     * Método responsável por receber dados desse periferico
     * 
     * @return String representando valor recebido do periferico
     * @exception java.lang.Exception
     */
    public String obterPeso() throws Exception {
        boolean isPesoEstavel = false;
        int tentativasLeitura = 0;
        String retorno = null;
        String peso = null;
        while (!isPesoEstavel) {
            tentativasLeitura++;
            peso = lerPeso(4); // this.getPortaSerial()
            // Balança ESTAVEL
            if (!peso.equals("-1")) { // PESO_INCORRETO
                isPesoEstavel = true;
                retorno = peso;
                // Balança OSCILANDO
            } else if (tentativasLeitura > 10) { // MAX_TENTATIVAS
                retorno = "0";
                break;
            }
        }
        return retorno;
    }

    public String numeroSerie() throws Exception {
        return obterNumeroSerie();
    }
}