package com.ccibm.ect.perifericos;

public class Test {

    public static void main (String[] args) {
        try {
            BalancaPadraoSara balanca = new BalancaPadraoSara();

            String numeroSerie = balanca.numeroSerie();
            System.out.println("Numero de série: " + numeroSerie);

            String peso = balanca.obterPeso();
            System.out.println("Peso: " + peso);

        } catch (Exception e ) {
            System.out.println(e.getMessage() + "Exciting");
            e.printStackTrace();
        }
    }
}