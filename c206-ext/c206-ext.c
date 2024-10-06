/*
 *  Předmět: Algoritmy (IAL) - FIT VUT v Brně
 *  Rozšíření pro příklad c206.c (Dvousměrně vázaný lineární seznam)
 *  Vytvořil: Daniel Dolejška, září 2024
 */

#include "c206-ext.h"

bool error_flag;
bool solved;

/**
 * Tato metoda simuluje příjem síťových paketů s určenou úrovní priority.
 * Přijaté pakety jsou zařazeny do odpovídajících front dle jejich priorit.
 * "Fronty" jsou v tomto cvičení reprezentovány dvousměrně vázanými seznamy
 * - ty totiž umožňují snazší úpravy pro již zařazené položky.
 * 
 * Parametr `packetLists` obsahuje jednotlivé seznamy paketů (`QosPacketListPtr`).
 * Pokud fronta s odpovídající prioritou neexistuje, tato metoda ji alokuje
 * a inicializuje. Za jejich korektní uvolnení odpovídá volající.
 * 
 * V případě, že by po zařazení paketu do seznamu počet prvků v cílovém seznamu
 * překročil stanovený MAX_PACKET_COUNT, dojde nejdříve k promazání položek seznamu.
 * V takovémto případě bude každá druhá položka ze seznamu zahozena nehledě
 * na její vlastní prioritu ovšem v pořadí přijetí.
 * 
 * @param packetLists Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param packet Ukazatel na strukturu přijatého paketu
 */
void receive_packet( DLList *packetLists, PacketPtr packet ) {
	// find apropriate queue
	QosPacketListPtr targetQueue = NULL;
	DLL_First(packetLists); // set first el as active
	while (DLL_IsActive(packetLists)) { // move through the whole list
		QosPacketListPtr cur_queue;
		DLL_GetValue(packetLists, (long *)&cur_queue);
		if (cur_queue->priority == packet->priority) {
			targetQueue = cur_queue;
			break;
		}
		DLL_Next(packetLists);
	}
	// if said queue does not exist -> create
	if (targetQueue == NULL) {
		targetQueue = (QosPacketListPtr) malloc (sizeof(QosPacketList));
		targetQueue->priority = packet->priority;
		targetQueue->list = (DLList *) malloc (sizeof(DLList));
		DLL_Init(targetQueue->list);
		DLL_InsertLast(packetLists, (long)targetQueue);
	}
	
	DLL_InsertLast(targetQueue->list, (long)packet);
	// overflow
	if (targetQueue->list->currentLength > MAX_PACKET_COUNT){
		DLL_First(targetQueue->list);
		int index = 1;
		while (DLL_IsActive(targetQueue->list)) {
			if (index % 2 == 0) {
				DLL_DeleteAfter(targetQueue->list);
			}
			DLL_Next(targetQueue->list);
			index++;
		}
	}
}

/**
 * Tato metoda simuluje výběr síťových paketů k odeslání. Výběr respektuje
 * relativní priority paketů mezi sebou, kde pakety s nejvyšší prioritou
 * jsou vždy odeslány nejdříve. Odesílání dále respektuje pořadí, ve kterém
 * byly pakety přijaty metodou `receive_packet`.
 * 
 * Odeslané pakety jsou ze zdrojového seznamu při odeslání odstraněny.
 * 
 * Parametr `packetLists` obsahuje ukazatele na jednotlivé seznamy paketů (`QosPacketListPtr`).
 * Parametr `outputPacketList` obsahuje ukazatele na odeslané pakety (`PacketPtr`).
 * 
 * @param packetLists Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param outputPacketList Ukazatel na seznam paketů k odeslání
 * @param maxPacketCount Maximální počet paketů k odeslání
 */
void send_packets( DLList *packetLists, DLList *outputPacketList, int maxPacketCount ) {
	int sent_ps = 0;
	DLL_First(packetLists);
	while (DLL_IsActive(packetLists) && sent_ps < maxPacketCount)
	{
		QosPacketListPtr curr_queue;
		DLL_GetValue(packetLists, (long *)&curr_queue);

		if (curr_queue->list->currentLength > 0) {
			PacketPtr pack_to_send;
			DLL_GetFirst(curr_queue->list, (long *)&pack_to_send);
			DLL_InsertLast(outputPacketList, (long)pack_to_send);
			sent_ps++;
			DLL_DeleteFirst(curr_queue->list);
			if (curr_queue->list->currentLength == 0) {};
		}
		DLL_Next(packetLists);
	}
}
