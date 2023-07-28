package com.ccibm.ect.perifericos;

import java.util.concurrent.TimeUnit;

public class Test {

    public static void main (String[] args) {
        try {
            BalancaPadraoSara balanca = new BalancaPadraoSara();

            String peso = balanca.obterPeso();
            System.out.println("Peso: " + peso + "\n");

            // peso = balanca.obterPeso();
            // System.out.println("Peso: " + peso + "\n");

            // TimeUnit.SECONDS.sleep(2);

            String numeroSerie = balanca.numeroSerie();
            System.out.println("Numero de série: " + numeroSerie + "\n");

        } catch (Exception e ) {
            System.out.println("Exciting with error: " + e.getMessage());
            // e.printStackTrace();
        }
    }
}