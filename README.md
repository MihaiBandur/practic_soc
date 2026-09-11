# Practic SOC – programe pentru ATmega1280

Colecție de programe C scrise pentru laboratorul / examenul practic de **Structura și Organizarea Calculatoarelor**. Toate programele rulează pe microcontrolerul **ATmega1280** (placa de laborator cu LED-uri, butoane și porturi mikroBUS) și sunt compilate cu **IAR Embedded Workbench for AVR**.

Fiecare folder de mai jos este descris prin: ce conține, ce problemă rezolvă, ce periferice / feature-uri ale procesorului folosește și ce subiecte acoperă.

---

## Context comun

| Element | Valoare |
|---|---|
| Microcontroler | ATmega1280 (AVR 8-bit, 128 KB flash, 8 KB SRAM) |
| Frecvență ceas (`F_CPU`) | 16 MHz |
| Compilator | IAR EW AVR (`ioavr.h`, `inavr.h`, `intrinsics.h`) |
| Baud rate serial | 9600, 8N1, `UBRR = F_CPU/16/BAUD - 1 = 103` |

Convenții IAR care apar peste tot:

- `#pragma vector = X_vect` + `__interrupt void f(void)` – definirea unei rutine de întrerupere (ISR)
- `__enable_interrupt()` / `__disable_interrupt()` – SEI / CLI
- `__delay_cycles(n)` – întârziere software
- `__watchdog_reset()` – instrucțiunea WDR
- `__sleep()` – instrucțiunea SLEEP
- `__flash` – constantă plasată în memoria program (tabelele CRC)
- `__farflash char *` – pointer pentru citirea memoriei flash peste 64 KB
- `__no_init` – variabilă în RAM care **nu** este inițializată la reset (își păstrează valoarea după un reset de watchdog sau de buton)

Pinii ATmega1280 folosiți în programe:

| Periferic | Pini | Unde apare |
|---|---|---|
| LED A / LED B | PA5 / PA6 | led, buton_led, debounce |
| Buton T1 / T4 (INT6) | PE6 | buton_led, debounce |
| INT0 | PD0 | software_interrupt, save_power_mode |
| OC0B (Timer0, PWM) | PG5 | fast_pwm_phase_correct, timer0_phase_correct |
| OC1A (Timer1, PWM) | PB5 | duty_cycle_modification |
| OC5B / OC5C (Timer5, PWM) | PL4 / PL5 | timer5_mod_15, timer5_mod15_pin_la_alegere |
| Intrare semnal măsurat | PF3 | periodmetru, frecventiometru, generator_de_semnale_usart |
| USART0 (RXD0/TXD0) | PE0 / PE1 | apasare_buton_reset, doua_apasari, duty_cycle, save_power, timp_seriala |
| USART1 (RXD1/TXD1) | PD2 / PD3 | lab_14_usart_watchdog |
| USART2 (RXD2/TXD2) | PH0 / PH1 | usart_neblocant, usart_crc, usart_flash_crc, crc_functie, watchdog_seriala, wdt_frecventa_usart_intreruperi |
| USART3 (RXD3/TXD3) | PJ0 / PJ1 | usart_blocant |

---

## Cuprins

| Folder | Temă principală | Subiecte |
|---|---|---|
| [16_ianuarie_2026](#1-16_ianuarie_2026) | GPIO, timere, PWM | LED, buton, debounce, PWM Timer0/Timer5, periodmetru |
| [20_ianuarie_2026](#2-20_ianuarie_2026) | CRC | CRC-16 și CRC-32 pe memoria flash, bit-cu-bit și cu tabel |
| [21_ianuarie_2026](#3-21_ianuarie_2026) | USART | USART blocant (polling) și neblocant (întreruperi) |
| [22_ianuarie_2025](#4-22_ianuarie_2025) | Întreruperi | Timer0 overflow, debounce, puls pe LED, INT0 „software” |
| [24 ianuarie 2026](#5-24-ianuarie-2026) | PWM parametrizabil, frecvențmetru | Timer0 phase correct, Timer5 mod 15 cu frecvență/duty setabile |
| [lab_14_usart_watchdog](#6-lab_14_usart_watchdog) | Driver USART + Watchdog | buffer circular, `my_print`, WDT în mod întrerupere + reset |
| [modele_practic](#7-modele_practic) | Modele de subiecte de examen | combinații de USART + timer + WDT + CRC + reset + sleep |

> Notă: folderul `22_ianuarie_2025` este cel mai probabil `22_ianuarie_2026` (greșeală de denumire), conținutul e din aceeași serie.

---

## 1. `16_ianuarie_2026`

**Ce rezolvă:** primele exerciții de bază – controlul unui LED, citirea unui buton, debounce, generarea de semnale PWM cu Timer0 și Timer5 și măsurarea perioadei unui semnal extern.

| Fișier | Ce face | Feature-uri ale procesorului | Subiect |
|---|---|---|---|
| `led.c` | Comută LED-ul de pe PA5 la fiecare 1 s (16 000 000 cicli). | GPIO: `DDRA`, `PORTA`; `__delay_cycles` | GPIO ieșire, blink cu întârziere software |
| `buton_led.c` | LED-ul PA5 se aprinde cât timp butonul de pe PE6 este apăsat (activ pe LOW). | GPIO intrare cu **pull-up intern** (`PORTE \|= 1<<PE6`), citire `PINE` | GPIO intrare, polling buton |
| `debounce.c` | Debounce software: la apăsarea butonului (INT6, front descrescător) se aprinde LED B, se dezactivează INT6 și se pornește un contor de 50 ms ținut de Timer0 (CTC, 1 ms). După 50 ms se reactivează INT6 și se comută LED A. | **Timer0 în mod CTC** (`WGM01`, prescaler 64, `OCR0A = 249` → 1 ms), întreruperea `TIMER0_COMPA_vect`; **întrerupere externă INT6** (`EICRB`/`ISC61`, `EIMSK`) | Debounce, timer ca ceas de sistem (millis), întreruperi externe |
| `fast_pwm_phase_correct.c` | PWM pe OC0B (PG5): Timer0 în mod 5 (**Phase Correct PWM, TOP = OCR0A**), prescaler 256, `OCR0A = 120`, `OCR0B = 84` → ~260 Hz, duty ~70 %. (Numele spune „fast”, dar configurația este phase correct.) | Timer0 PWM: `WGM00`, `WGM02`, `COM0B1`, `CS02` | PWM 8-bit, TOP setabil prin OCR0A |
| `periodmetru.c` | Măsoară perioada semnalului de pe PF3: așteaptă un front (sacrificat), pornește Timer1 fără prescaler, așteaptă o perioadă completă, oprește timerul și calculează `perioada_us` și `frecventa`. Overflow-urile Timer1 sunt numărate în ISR. | **Timer1 în mod Normal** (`CS10`), `TIMER1_OVF_vect`, `TCNT1`; polling pe `PINF` | Periodmetru / frecvențmetru prin polling de fronturi |
| `timer5_mod_15.c` | Timer5 în **mod 15 (Fast PWM, TOP = OCR5A)**, prescaler 8, ieșiri OC5B/OC5C (PL4/PL5). La fiecare overflow ISR-ul comută PE5 (pentru osciloscop) și alternează `OCR5A` între 1775 și 340, deci frecvența se schimbă la fiecare perioadă (~1,1 kHz ↔ ~5,9 kHz). | Timer5 16-bit: `WGM50..53`, `COM5B1`, `COM5C1`, `TIMSK5/TOIE5` | PWM 16-bit, schimbarea frecvenței din ISR |

---

## 2. `20_ianuarie_2026`

**Ce rezolvă:** verificarea integrității memoriei program prin CRC. Se calculează CRC-ul întregii memorii flash (de la 0 până la ultimii 2 / 4 octeți) și se compară cu valoarea stocată de linker la sfârșitul flash-ului.

| Fișier | Ce face | Feature-uri ale procesorului | Subiect |
|---|---|---|---|
| `crc16.c` | Implementează CRC-16 (polinom `0x1021` MSB-first, `0x8408` LSB-first) în două variante: `crc16()` bit-cu-bit și `crc16wtable()` cu tabel de 256 de intrări. Calculează CRC pe `[0, 0x20000-2)` și îl compară cu cel citit de la `0x20000-2`. | Citirea memoriei **flash** cu `__farflash char *`; tabele plasate în flash cu `__flash const` | CRC-16, lookup table, ordinea biților (MSBF/LSBF) |
| `crc32.c` | Același lucru pentru CRC-32 (polinom `0x04C11DB7` / `0xEDB88320`), cu funcțiile `crc32_f()` și `crc32wtable()`. | idem | CRC-32 |

Aceste funcții sunt refolosite identic în `modele_practic` (`crc_functie.c`, `usart_crc.c`, `usart_flash_crc.c`).

---

## 3. `21_ianuarie_2026`

**Ce rezolvă:** comunicația serială: cum se inițializează un USART și cum se face un „echo” (ce primești trimiți înapoi) în varianta blocantă și în varianta cu întreruperi.

| Fișier | Ce face | Feature-uri ale procesorului | Subiect |
|---|---|---|---|
| `usart_blocant.c` | Echo pe **USART3** (PJ0/PJ1), 9600 baud. `usart_transmit()` așteaptă `UDRE3`, `usart_receive()` așteaptă `RXC3` prin polling. | `UBRR3H/L`, `UCSR3A/B`, `UDR3`, `TXEN3`, `RXEN3` | USART blocant (polling) |
| `usart_neblocant.c` | Echo pe **USART2** (PH0/PH1) cu întreruperi: `USART2_RX_vect` salvează caracterul și setează un flag, `USART2_UDRE_vect` îl retransmite. Întreruperea RX este dezactivată pe durata transmisiei și reactivată apoi. | `RXCIE2`, `UDRIE2`, ISR-uri RX și UDRE | USART neblocant (întreruperi), handshake între ISR-uri prin flag |

---

## 4. `22_ianuarie_2025`

**Ce rezolvă:** exerciții de întreruperi: timer overflow, întreruperi externe și generarea unei întreruperi „software”.

| Fișier | Ce face | Feature-uri ale procesorului | Subiect |
|---|---|---|---|
| `blinking_led.c` | Timer0 în mod Normal cu prescaler 1024; la fiecare 30 de overflow-uri (~0,5 s) se inversează tot `PORTD`. | `TCCR0B` (`CS02\|CS00`), `TIMSK0/TOIE0`, `TIMER0_OVF_vect` | Blink LED cu timer + întrerupere, fără delay |
| `debounce.c` | Identic cu `16_ianuarie_2026/debounce.c` (INT6 + Timer0 CTC 1 ms). | vezi mai sus | Debounce |
| `interrupt_led_pulse.c` | La fiecare overflow al Timer0 (prescaler 64, ~1 ms) generează un puls scurt (~10 000 cicli) pe PD7 (LED) și pe PC0 (semnal de referință pentru osciloscop). | Timer0 overflow, `__delay_cycles` în ISR | Puls periodic, sincronizare/debug cu osciloscop |
| `software_interrupt.c` | Configurează INT0 (PD0) pe front crescător. În `main` se inversează continuu `PORTD`, deci PD0 (care este chiar pinul INT0, setat ca ieșire) generează singur fronturi și declanșează ISR-ul, care incrementează un contor static. | `EICRA` (`ISC01\|ISC00`), `EIMSK/INT0`, `INT0_vect` | Întreruperi externe, întrerupere declanșată „software” prin propriul pin |

---

## 5. `24 ianuarie 2026`

**Ce rezolvă:** versiuni parametrizabile ale PWM-ului (frecvență și duty cycle date în Hz și %) și frecvențmetrul.

| Fișier | Ce face | Feature-uri ale procesorului | Subiect |
|---|---|---|---|
| `frecventiometru.c` | Identic cu `periodmetru.c`: măsoară perioada și frecvența semnalului de pe PF3 cu Timer1. | Timer1 Normal + overflow ISR | Frecvențmetru |
| `timer0_phase_correct.c` | Timer0 în **Phase Correct PWM, TOP = OCR0A**, OC0B pe PG5, prescaler 256. `set_frequency(f)` calculează `TOP = F_CPU / (2·256·f)`, `set_duty_cycle(%)` calculează `OCR0B = % · OCR0A / 100`. Exemplu: 500 Hz, 35 %. | Timer0 PWM mod 5, `COM0B1` | PWM cu frecvență și duty configurabile, formula pentru phase correct |
| `timer5_mod15_pin_la_alegere.c` | Timer5 în **mod 15 (Fast PWM, TOP = OCR5A)**, prescaler 8, OC5B/OC5C. `timer5_set_frequency(f)` → `TOP = F_CPU/(8·f) - 1`; `timer5_set_duty(%, 'B'/'C')` alege canalul. ISR-ul de overflow alternează 1 kHz ↔ 5 kHz și comută PE5. Scrierile în OCR5x sunt protejate cu `__disable_interrupt()` (secțiune critică pentru registre 16-bit). | Timer5 mod 15, `OCR5A/B/C`, `TOIE5`, secțiuni critice | PWM 16-bit configurabil per canal, formula pentru fast PWM |

---

## 6. `lab_14_usart_watchdog`

**Ce rezolvă:** un mic driver USART reutilizabil (bazat pe buffere circulare și întreruperi), o funcție `my_print` de tip printf simplificat și un exemplu de folosire a **Watchdog Timer**-ului. Este singurul folder cu proiect multi-fișier.

| Fișier | Ce face | Feature-uri ale procesorului | Subiect |
|---|---|---|---|
| `round_buff.h/.c` | Buffer circular de 16 octeți cu `push`, `pop`, `push_vec`, `is_empty`, `is_full` (indici `head`/`tail`, aritmetică modulo). | – (structură de date) | Buffer circular (ring buffer) |
| `usart.h/.c` | Driver **USART1** (PD2/PD3), 9600 baud, 8 biți, **paritate pară** (`UPM11`). Transmisia se face din `tx_buffer` în `USART1_TX_vect`, recepția în `rx_buffer` din `USART1_RX_vect`. Funcții: `USART_initialize`, `USART_transmit_char/string`, `USART_receive_char` (blocantă), `USART_receive_string`. Accesul la buffere e protejat cu `__disable_interrupt()`. | `UCSR1A/B/C`, `RXCIE1`, `TXCIE1`, `UDR1` | USART cu întreruperi + buffere, secțiuni critice |
| `mylib.h/.c` | `my_print(tip, &val)` – convertește manual un `int`, `hex`, `double` (2 zecimale) sau `char` în șir de caractere și îl trimite pe serială. | – | Conversie număr → text fără `printf` |
| `main.c` | Trimite „8.789”, apoi configurează **WDT cu timeout ~4 s** (`WDP3`) în mod *Interrupt + System Reset*. În bucla principală face `__watchdog_reset()` și echo la caracterele primite. Dacă nu vine niciun caracter timp de ~4 s, ISR-ul `WDT_vect` trimite `!`; dacă mai trec ~4 s, microcontrolerul se resetează. | **Watchdog**: `WDTCSR`, secvența `WDCE\|WDE`, `WDIE`, `WDP3`, `MCUSR = 0`, `__watchdog_reset()` | Watchdog în mod întrerupere și reset, secvența temporizată de configurare |

---

## 7. `modele_practic`

**Ce rezolvă:** modele de subiecte pentru examenul practic. Fiecare fișier combină 2–3 periferice (de obicei USART + timer + watchdog/CRC/reset) și trimite rezultatul pe serială. Sunt toate proiecte single-file, cu transmisie serială pe întreruperea UDRE dintr-un buffer text.

### 7.1 Timer + USART (cronometre, PWM)

| Fișier | Ce face | Feature-uri ale procesorului | Subiect |
|---|---|---|---|
| `apasare_buton_reset.c` | Cronometru de la ultimul reset: Timer1 CTC (prescaler 8, `OCR1A = 1999`) numără tick-uri; la fiecare 50 de tick-uri formatează „S.SS secunde” și pornește transmisia pe USART0 prin `USART0_UDRE_vect`. *(schiță: are erori de sintaxă – `ubrr` nedefinit, `UCSR0C = \|`, `;` lipsă, `=` în loc de `!=`)* | Timer1 CTC (`WGM12`, `OCIE1A`), USART0 TX cu UDRE | Uptime, conversie număr → text, TX pe întreruperi |
| `doua_apasari_buton_reset.c` | Măsoară timpul dintre **două apăsări ale butonului RESET**. Contorul `timer_sutimi` este `__no_init`, deci supraviețuiește resetului. La pornire citește `MCUSR`; dacă bitul `EXTRF` (reset extern) e setat, trimite pe USART0 „S.SS” (cu `sprintf`), apoi repornește contorul (Timer1 CTC, 10 ms). | `MCUSR`/`EXTRF` (sursa resetului), `__no_init`, Timer1 CTC, USART0 UDRE | Surse de reset, păstrarea datelor peste reset |
| `duty_cycle_modification.c` | PWM 10 kHz pe OC1A (PB5) cu Timer1 în **mod 14 (Fast PWM, TOP = ICR1 = 1599)**. ISR-ul de overflow numără 278 perioade (~27,8 ms), apoi modifică duty-ul în ping-pong 5 % → 95 % → 5 % cu pas 5 și trimite „<FU=xx%>” pe USART0. | Timer1 mod 14 (`WGM11..13`, `ICR1`, `COM1A1`), `TOIE1`, USART0 UDRE, `sprintf` | PWM cu TOP în ICR1, modificarea duty-ului în timp real, raportare serială |
| `timer5_fast_pwm_modificabil.c` | PWM „software” pe un pin oarecare (PE5): Timer5 mod 15 (`OCR5A = 1666`, prescaler 8, ~1,2 kHz); ISR-ul de overflow pune pinul pe 1, ISR-ul `TIMER5_COMPB_vect` (`OCR5B = 500`, ~30 %) îl pune pe 0. | Timer5 mod 15, `TOIE5`, `OCIE5B` | PWM pe pin fără ieșire OCnx, folosind două întreruperi |
| `generator_de_semnale_usart.c` | Periodmetrul din `16_ianuarie_2026` (PF3, Timer1) rulat de 10 ori, apoi trimite „T = … us \| F = … Hz” pe USART2 prin UDRE. Conține `ul_to_dec()` pentru conversie `unsigned long` → text. | Timer1 Normal + overflow, USART2 UDRE | Frecvențmetru + raportare pe serială |
| `timp_seriala.c` | Măsoară cât durează transmisia unui șir pe USART0: la primirea unei taste (RX ISR) pornește Timer1 (prescaler 64, 4 µs/tick) și transmisia pe UDRE; când se golește șirul activează întreruperea **TX Complete** (`TXCIE0`), care oprește timerul; apoi trimite „Timp: … us”. *(schiță: are typo-uri – `instrincs.h`, `TCTN1`, nume de variabile inconsistente)* | USART0 RX / UDRE / **TXC**, Timer1 free-running | Diferența dintre UDRE și TXC, măsurarea duratei unei transmisii |

### 7.2 Watchdog

| Fișier | Ce face | Feature-uri ale procesorului | Subiect |
|---|---|---|---|
| `watchdog_frec.c` | Folosește WDT-ul ca timer în **mod întrerupere** (16 ms, `WDP = 0000`) pentru a genera pe PB0 un semnal cu HIGH 8 tick-uri (~128 ms) și LOW 4 tick-uri (~64 ms) → ~5 Hz, duty ~66 %. *(typo: `__enable_interrrupt`, `WDT_ISR(WDT_vect)`)* | `WDTCSR`: `WDCE\|WDE` apoi `WDIE=1, WDE=0`; `WDT_vect` | WDT ca sursă de timp, generare semnal |
| `watchdog_frecvetnta_20Hz.c` | Același principiu: tick de 16 ms; pinul PB0 e pus pe LOW la tick 1 și pe HIGH la tick 3 → perioadă 48 ms (~20,8 Hz). *(typo: `WDT_vector`, nume de funcție inconsistent)* | WDT mod întrerupere | Semnal ~20 Hz cu WDT |
| `watchdog_seriala.c` | Măsoară **frecvența reală a oscilatorului WDT** folosind resetul: Timer1 rulează liber (prescaler 1), valoarea `TCNT1` și numărul de overflow-uri sunt ținute în variabile `__no_init`. WDT în mod reset la 32 ms (`WDP1\|WDP0` = 4096 cicli). După reset, din tick-urile salvate se calculează perioada și `f_WDT = 4096000 / T_us` kHz, trimisă pe USART2 ca „WDT FREQ = … kHz”. | WDT mod reset, `__no_init`, `MCUSR`, Timer1 overflow, `asm("WDR")`, USART2 UDRE | Măsurarea perioadei WDT prin reset, date păstrate peste reset |
| `wdt_frecventa_usart_intreruperi.c` | Aceeași măsurătoare, dar **fără reset**: WDT în mod întrerupere (32 ms). În `WDT_vect` se citește `TCNT1` + overflow-uri, se resetează timerul, se calculează frecvența în kHz și se trimite „WDT= … kHz” pe USART2. `main` doar apelează `__sleep()`. | WDT mod întrerupere, Timer1, USART2 UDRE, `__sleep()` | Măsurarea frecvenței WDT cu întreruperi |
| `save_power_mode.c` | La apăsarea butonului INT0 (PD0) pornește Timer1 și WDT în mod întrerupere (16 ms, 2048 cicli); între două întreruperi WDT se măsoară tick-urile, se calculează frecvența WDT și se trimite ca „X.XXX MHz” pe USART0. Apoi procesorul intră în **Power-save** (`SMCR`: SM2..0 = 011, SE = 1, `__sleep()`), iar INT0 este reactivat la sfârșitul transmisiei. *(schiță: `WDCTSR`, `INT0__vect`, `TIMER0_OVF_vect` în loc de `TIMER1`, nume de funcții inconsistente)* | INT0, WDT întrerupere, Timer1, USART0, **sleep modes** (`SMCR`, `__sleep()`) | Moduri de consum redus, măsurare WDT declanșată de buton |

### 7.3 CRC + USART

| Fișier | Ce face | Feature-uri ale procesorului | Subiect |
|---|---|---|---|
| `crc_functie.c` | Calculează CRC-16 (tabel, MSBF) peste codul unei funcții din flash: intervalul `[&min, &dummy_function)`. Trimite pe USART2 „CRC16 ADR [0xAAAA - 0xBBBB] 0xCCCC” (adrese și CRC în hex, conversie manuală cu `u16_to_hex`). | `__farflash`, `__flash`, pointeri la funcții ca adrese de flash, USART2 UDRE | CRC pe o zonă de cod, formatare hex |
| `usart_crc.c` | Primește caractere pe USART2 până la `@`, calculează CRC-16 peste mesajul primit și îl retrimite cu sufixul „ CRC160_xHHHH”. *(atenție: `crc16wtable` citește prin `__farflash`, deci pe un buffer din RAM rezultatul nu e cel corect; pentru RAM trebuie o variantă cu pointer normal)* | USART2 RX/UDRE, tabel CRC-16 | CRC pe date primite pe serială, protocol cu terminator |
| `usart_flash_crc.c` | Protocol pe USART2: se primesc două adrese hex de 4 caractere (separate de un caracter non-hex), se validează (`hex_string_to_uint16`, `is_flash_address_valid`, `end >= start`), se calculează CRC-16 peste flash `[start, end]` și se trimit înapoi cele 4 cifre hex ale CRC-ului. Recepția este o mică mașină de stări (`address_number`). | USART2 RX/UDRE, `__farflash`, `strchr`, validare intrări | CRC pe interval de flash dat de utilizator, FSM de parsare serială |

---

## Hartă subiecte → fișiere

| Subiect | Fișiere |
|---|---|
| GPIO (LED, buton, pull-up) | `16/led.c`, `16/buton_led.c` |
| Debounce | `16/debounce.c`, `22/debounce.c` |
| Întreruperi externe (INT0, INT6) | `22/software_interrupt.c`, `16/debounce.c`, `modele/save_power_mode.c` |
| Timer0 (8-bit): Normal / CTC / PWM | `22/blinking_led.c`, `22/interrupt_led_pulse.c`, `16/debounce.c`, `16/fast_pwm_phase_correct.c`, `24/timer0_phase_correct.c` |
| Timer1 (16-bit): măsurare perioadă / CTC / PWM mod 14 | `16/periodmetru.c`, `24/frecventiometru.c`, `modele/generator_de_semnale_usart.c`, `modele/doua_apasari_buton_reset.c`, `modele/duty_cycle_modification.c` |
| Timer5 (16-bit): PWM mod 15 | `16/timer5_mod_15.c`, `24/timer5_mod15_pin_la_alegere.c`, `modele/timer5_fast_pwm_modificabil.c` |
| USART blocant | `21/usart_blocant.c` |
| USART cu întreruperi (RX, UDRE, TXC) | `21/usart_neblocant.c`, `lab_14/usart.c`, toate din `modele_practic` |
| Buffer circular, `my_print` | `lab_14/round_buff.c`, `lab_14/mylib.c` |
| Watchdog mod reset | `lab_14/main.c`, `modele/watchdog_seriala.c` |
| Watchdog mod întrerupere | `modele/watchdog_frec.c`, `modele/watchdog_frecvetnta_20Hz.c`, `modele/wdt_frecventa_usart_intreruperi.c`, `modele/save_power_mode.c` |
| Surse de reset (`MCUSR`), `__no_init` | `modele/doua_apasari_buton_reset.c`, `modele/watchdog_seriala.c` |
| Sleep / power-save (`SMCR`) | `modele/save_power_mode.c` |
| CRC-16 / CRC-32 | `20/crc16.c`, `20/crc32.c`, `modele/crc_functie.c`, `modele/usart_crc.c`, `modele/usart_flash_crc.c` |
| Citire flash (`__flash`, `__farflash`) | `20/*`, `modele/crc_*.c`, `modele/usart_flash_crc.c` |

---

## Formule utile (F_CPU = 16 MHz)

| Ce | Formulă |
|---|---|
| Baud rate | `UBRR = F_CPU / (16 · BAUD) - 1` → 103 pentru 9600 |
| Timer CTC, întrerupere la T | `OCR = F_CPU / (prescaler · f) - 1` (ex. 1 ms, prescaler 64 → 249) |
| Fast PWM (mod 14/15), frecvență | `TOP = F_CPU / (prescaler · f) - 1` |
| Phase Correct PWM, frecvență | `TOP = F_CPU / (2 · prescaler · f)` |
| Duty cycle | `OCRnX = duty% · TOP / 100` |
| Perioadă din tick-uri Timer1 (prescaler 1) | `T_us = (ovf · 65536 + TCNT1) / 16` |
| Frecvență WDT | `f_WDT[kHz] = cicli_WDT · 1000 / T_us` (2048 cicli pentru WDP=0000, 4096 pentru WDP=0011) |
