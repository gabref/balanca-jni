package com.ccibm.ect.perifericos;

public class Test {

    public static void main (String[] args) {
        try {
            BalancaPadraoSara balanca = new BalancaPadraoSara();

            String peso = balanca.obterPeso();
            System.out.println("Peso: " + peso + "\n");

            peso = balanca.obterPeso();
            System.out.println("Peso: " + peso + "\n");

            String numeroSerie = balanca.numeroSerie();
            System.out.println("Numero de serie: " + numeroSerie + "\n");

        } catch (Exception e ) {
            System.out.println("Exciting with error: " + e.getMessage());
            // e.printStackTrace();
        }
    }
}