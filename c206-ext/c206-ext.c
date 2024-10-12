/*
 *  Předmět: Algoritmy (IAL) - FIT VUT v Brně
 *  Rozšíření pro příklad c206.c (Dvousměrně vázaný lineární seznam)
 *  Vytvořil: Daniel Dolejška, září 2024
 */

#include "c206-ext.h"

bool error_flag;
bool solved;

/** This function is used to print content of the queue
 * Is a similar function used in tests? Probably...
 *  * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void printQueue(DLList *list) { // for debugging
    DLLElementPtr current = list->firstElement;
    int counter = 1;
    printf("Queue report:\n");
    while (current != NULL) {
        printf("\t%d packet(s)  received \n", counter);
        current = current->nextElement;
        counter++;
    }
}
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
    DLLElementPtr queue_ptr = packetLists->firstElement; // first element pointing to the first queue
    bool IsQueue_supplied = false;

    while (queue_ptr != NULL) { // element not empty => it contains a queue
        QosPacketListPtr qos_list = (QosPacketListPtr)queue_ptr->data; // Get the QoS list (prio and list)
        
        if (qos_list->priority == packet->priority) {
            DLList *qpacket_list = qos_list->list;
            if (qpacket_list->currentLength >= MAX_PACKET_COUNT) {
                int count = 0;
                DLL_First(qpacket_list);
                while (DLL_IsActive(qpacket_list)) { //remove every second packet
                    if  (count % 2 == 0) {
                        DLL_DeleteAfter(qpacket_list); 
                        //printf("removed packet\n"); // DEBUG
                    }
                    else DLL_Next(qpacket_list);
                    count++;
                }
                DLL_Last(qpacket_list);
            }
            
            // printf("Inserting packet %d with priority %d into existing queue with priority %d\n", packet->id, packet->priority, qos_list->priority); // DEBUG
            DLL_InsertLast(qpacket_list, (long)packet); // Insert the packet pointer into the list
            IsQueue_supplied = true;
            // printQueue(qpacket_list); // DEBUG
            break;
        }
        queue_ptr = queue_ptr->nextElement; // prio check failed, move to next element(containing queue)
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
        // printf("Inserting packet %d with priority %d into new queue with priority %d\n", packet->id, packet->priority, queue_new->priority); // DEBUG
        //printQueue(queue_new->list); // DEBUG
    }
}
/**
 * This function sorts queues based on their priority
 * This means the highest priority queue will be first,second-highest second, etc.
 * Bubble sort was chosen as sorting algorithm for its simplicity.
 * Of course, for larger ammounts of queues, any other algorithm would be better than this...
 * 
 * @param packetLists Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void queue_sorter (DLList *packetLists) {
    if (packetLists == NULL || packetLists->firstElement == NULL) {
        return; // empty packet list, do nothing
    }
    bool swapped;
    DLLElementPtr actual;
    QosPacketListPtr actual_queue, next_queue;

    do {
        swapped = false;
        actual = packetLists->firstElement;

        while (actual->nextElement != NULL) 
        {
            actual_queue = (QosPacketListPtr) actual->data;
            next_queue = (QosPacketListPtr) actual->nextElement->data;
            if (actual_queue->priority < next_queue->priority) 
            {
                actual->data = (long)next_queue; // store first
                actual->nextElement->data = (long)actual_queue; // store second
                swapped = true;
            }
            actual = actual->nextElement;
        }
    }while(swapped);
}
/*
c206-ext.c: In function ‘queue_sorter’:
c206-ext.c:114:36: warning: unused variable ‘next_queue’ [-Wunused-variable]
  114 |     QosPacketListPtr actual_queue, next_queue;
      |                                    ^~~~~~~~~~
c206-ext.c:114:22: warning: unused variable ‘actual_queue’ [-Wunused-variable]
  114 |     QosPacketListPtr actual_queue, next_queue;
      |                      ^~~~~~~~~~~~
*/
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
    queue_sorter(packetLists); //sort queues before starting
	DLL_First(packetLists);
	while (DLL_IsActive(packetLists) && sent_ps < maxPacketCount) // move inside the list of queues
	{
		QosPacketListPtr curr_queue;
		DLL_GetValue(packetLists, (long *)&curr_queue); // get q from packetlists

		while (DLL_IsActive(curr_queue->list) && sent_ps < maxPacketCount) { //move inside the queue
            PacketPtr pack_to_send;
            DLL_GetFirst(curr_queue->list, (long *)&pack_to_send); // get packet (rec_q->list)
            DLL_InsertLast(outputPacketList, (long)pack_to_send); // store packet (out_q->list)
            DLL_DeleteFirst(curr_queue->list); // remove original packet
            DLL_First(curr_queue->list); // guess who forgot DeleteFirst can remove activity
            sent_ps++; // increment sent packets count
        }

        if (curr_queue->list->currentLength == 0) {
            DLL_Dispose(curr_queue->list); // remove empty list
        }
        DLL_Next(packetLists); // move to the next queue if there's still an active element
    }
}