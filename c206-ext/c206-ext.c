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

    DLLElementPtr queue_actual = packetLists->firstElement; // first queue
    bool IsQueue_supplied = false;

    // Attempt to find a queue with the matching priority
    while (queue_actual != NULL) {
        QosPacketListPtr qosList = (QosPacketListPtr)queue_actual->data; // Get the QoS list
        if (qosList->priority == packet->priority) {
            DLList *qpacket_list = qosList->list;
            DLL_Init(qpacket_list);
            
            DLL_InsertLast(qpacket_list, (long)packet); // Insert the packet pointer into the list
            IsQueue_supplied = true;
            break;
        }
        queue_actual = queue_actual->nextElement; // Move on to the next queue
    }

    // If no queue found, create a new one
    if (!IsQueue_supplied) {
        QosPacketListPtr queue_new = (QosPacketListPtr)malloc(sizeof(QosPacketList)); // new queue
        if (queue_new == NULL) {
            printf("Failed to allocate memory for new queue\n");
            return;
        }
        
        queue_new->list = (DLList*)malloc(sizeof(DLList));
        if (queue_new->list == NULL) { // Memory allocation failure
            free(queue_new);
            printf("Failed to allocate memory for new queue list\n");
            return;
        }
        
        DLL_Init(queue_new->list);
        queue_new->priority = packet->priority; // Set priority from packet

        DLL_InsertLast(packetLists, (long)queue_new); // Insert the new queue list into packetLists

        DLL_InsertLast(queue_new->list, (long)packet); // Insert the packet pointer into the list
        DLL_First(queue_new->list);
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