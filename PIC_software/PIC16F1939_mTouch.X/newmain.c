/* 
 * File:   newmain.c
 * Author: Mogami
 *
 * Created on 2020/02/18, 22:13
 */

#include <stdio.h>
#include <stdlib.h>
#define _XTAL_FREQ 32000000
#pragma config FOSC = INTOSC       // Oscillator Selection (ECH, External Clock, High Power Mode (4-32 MHz): device clock supplied to CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config VCAPEN = OFF     // Voltage Regulator Capacitor Enable (All VCAP pin functionality is disabled)
#pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)

#include <xc.h>


#define ROBE_NUMBER 13              // 接続している電極の個数を指定

unsigned int CPS_data[2][ROBE_NUMBER]; // 容量検知用データ
/*
 * 
 */
void CPS_Init() {
    int i;

    // 接続している電極の分だけ繰り返す
    for (i = 0; i < ROBE_NUMBER; i++) {
        CPSCON1 = i; // 読み込むチャンネルを設定する
        TMR1H = 0; // タイマー1の初期化
        TMR1L = 0;
        CPSON = 1; // 容量検知モジュール開始
        __delay_us(5000); // 5msの間タイマー1をカウントさせる
        // 容量検知モジュールの値を読み込む
        CPSON = 0; // 容量検知モジュール停止
        CPS_data[0][i] = (TMR1H * 256) + TMR1L;
        CPS_data[1][i] = 0;
    }
}
// 容量検知モジュールに接続されている電極の現在値を読み込む処理

void CPS_ScanRobe() {
    unsigned int cap;
    int i;

    // 接続している電極の分だけ繰り返す
    for (i = 0; i < ROBE_NUMBER; i++) {
        CPSCON1 = i; // 読み込むチャンネルを設定する
        TMR1H = 0; // タイマー1の初期化
        TMR1L = 0;
        CPSON = 1; // 容量検知モジュール開始
        __delay_us(5000); // 5msの間タイマー1をカウントさせる
        // 容量検知モジュールの値を読み込む
        CPSON = 0; // 容量検知モジュール停止
        cap = (TMR1H * 256) + TMR1L;
        if (cap <= (CPS_data[0][i]*0.9)) {
            CPS_data[1][i] = cap; // ONとする
        } else {
            CPS_data[1][i] = 0; // OFFとする
            CPS_data[0][i] = cap;
        }
    }
}
// 容量検知モジュールに接続されている電極の状態を調べる処理
//   num : 調べる電極の番号を指定する

int CPS_StateRobe(int num) {
    if (num > ROBE_NUMBER) return ( -1); // 数値指定エラー
    if (CPS_data[1][num - 1] == 0) return ( 0); // 電極に触れていない
    else return ( 1); // 電極に触れている
}

int main(int argc, char** argv) {
    OSCCON = 0b01110000;
    ANSELA = 0b00110000;
    TRISA = 0b00110000;
    ANSELB = 0b00011111;
    TRISB = 0b00011111;
    TRISC = 0b00000000;
    ANSELD = 0b00011111;
    TRISD = 0b00011111;

    PORTA = 0b00000000;
    PORTB = 0b00000000;
    PORTC = 0b00000000;
    PORTD = 0b00000000;
    // 容量検知モジュール(ＣＰＳＭ)の設定
    CPSCON0 = 0b00001100; // オシレータは高範囲(高速の発信周波数)で利用する
    // タイマー１の設定
    T1CON = 0b11000001; // 容量検知オシレータでTIMER1をｶｳﾝﾄする、ﾌﾟﾘｽｹｰﾗｶｳﾝﾄ値 1:1
    TMR1H = 0; // タイマー1の初期化
    TMR1L = 0;
    PEIE = 1; // 周辺装置割り込みを許可する
    GIE = 1; // 全割り込み処理を許可する 

    // ＵＳＡＲＴ機能の設定を行う
    //TXSTA = 0b00101110; 
    TXSTA  = 0b00100100 ;// 送信情報設定：非同期モード　８ビット・ノンパリティ
    RCSTA = 0b10010000; // 受信情報設定
    SYNC = 0;
    BRGH = 0;
    BRG16 = 0;
    SPBRG = 51; //(32M ÷　9600 ÷ 64 )-1

    // 容量検知モジュールの初期値を読み込む
    CPS_Init();
    while (1) {
        // 容量検知モジュールの現在値を読み込む
        CPS_ScanRobe();
        // 電極１(CPS0)の状態でＬＥＤ１を点灯させる処理
        //if (CPS_StateRobe(4) == 1) RC2 = 1; // LED1を点灯
        //else RC2 = 0; // LED1を消灯
        
        for(int i =1;i<14;i++){
            while(TXIF==0) ;
            if (CPS_StateRobe(i) == 1){
                TXREG = 0x30 + i;
            }
        }
        // 電極２(CPS1)の状態でＬＥＤ２を点灯させる処理
        //if (CPS_StateRobe(2) == 1) RC2 = 1; // LED2を点灯
        //else RC2 = 0; // LED2を消灯
    }
    return (EXIT_SUCCESS);
}