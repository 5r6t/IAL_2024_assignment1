/* ******************************* c206.c *********************************** */
/*  Předmět: Algoritmy (IAL) - FIT VUT v Brně                                 */
/*  Úkol: c206 - Dvousměrně vázaný lineární seznam                            */
/*  Návrh a referenční implementace: Bohuslav Křena, říjen 2001               */
/*  Vytvořil: Martin Tuček, říjen 2004                                        */
/*  Upravil: Kamil Jeřábek, září 2020                                         */
/*           Daniel Dolejška, září 2021                                       */
/*           Daniel Dolejška, září 2022                                       */
/* ************************************************************************** */
/*
** Implementujte abstraktní datový typ dvousměrně vázaný lineární seznam.
** Užitečným obsahem prvku seznamu je hodnota typu int. Seznam bude jako datová
** abstrakce reprezentován proměnnou typu DLList (DL znamená Doubly-Linked
** a slouží pro odlišení jmen konstant, typů a funkcí od jmen u jednosměrně
** vázaného lineárního seznamu). Definici konstant a typů naleznete
** v hlavičkovém souboru c206.h.
**
** Vaším úkolem je implementovat následující operace, které spolu s výše
** uvedenou datovou částí abstrakce tvoří abstraktní datový typ obousměrně
** vázaný lineární seznam:
**
**      DLL_Init ........... inicializace seznamu před prvním použitím,
**      DLL_Dispose ........ zrušení všech prvků seznamu,
**      DLL_InsertFirst .... vložení prvku na začátek seznamu,
**      DLL_InsertLast ..... vložení prvku na konec seznamu,
**      DLL_First .......... nastavení aktivity na první prvek,
**      DLL_Last ........... nastavení aktivity na poslední prvek,
**      DLL_GetFirst ....... vrací hodnotu prvního prvku,
**      DLL_GetLast ........ vrací hodnotu posledního prvku,
**      DLL_DeleteFirst .... zruší první prvek seznamu,
**      DLL_DeleteLast ..... zruší poslední prvek seznamu,
**      DLL_DeleteAfter .... ruší prvek za aktivním prvkem,
**      DLL_DeleteBefore ... ruší prvek před aktivním prvkem,
**      DLL_InsertAfter .... vloží nový prvek za aktivní prvek seznamu,
**      DLL_InsertBefore ... vloží nový prvek před aktivní prvek seznamu,
**      DLL_GetValue ....... vrací hodnotu aktivního prvku,
**      DLL_SetValue ....... přepíše obsah aktivního prvku novou hodnotou,
**      DLL_Previous ....... posune aktivitu na předchozí prvek seznamu,
**      DLL_Next ........... posune aktivitu na další prvek seznamu,
**      DLL_IsActive ....... zjišťuje aktivitu seznamu.
**
** Při implementaci jednotlivých funkcí nevolejte žádnou z funkcí
** implementovaných v rámci tohoto příkladu, není-li u funkce explicitně
** uvedeno něco jiného.
**
** Nemusíte ošetřovat situaci, kdy místo legálního ukazatele na seznam
** předá někdo jako parametr hodnotu NULL.
**
** Svou implementaci vhodně komentujte!
**
** Terminologická poznámka: Jazyk C nepoužívá pojem procedura.
** Proto zde používáme pojem funkce i pro operace, které by byly
** v algoritmickém jazyce Pascalovského typu implemenovány jako procedury
** (v jazyce C procedurám odpovídají funkce vracející typ void).
**
**/

#include "c206.h"

bool error_flag;
bool solved;

/**
 * Vytiskne upozornění na to, že došlo k chybě.
 * Tato funkce bude volána z některých dále implementovaných operací.
 */
void DLL_Error(void) {
	printf("*ERROR* The program has performed an illegal operation.\n");
	error_flag = true;
}

/**
 * Provede inicializaci seznamu list před jeho prvním použitím (tzn. žádná
 * z následujících funkcí nebude volána nad neinicializovaným seznamem).
 * Tato inicializace se nikdy nebude provádět nad již inicializovaným seznamem,
 * a proto tuto možnost neošetřujte.
 * Vždy předpokládejte, že neinicializované proměnné mají nedefinovanou hodnotu.
 *
 * @param list Ukazatel na strukturu dvousměrně vázaného seznamu
 */
void DLL_Init( DLList *list ) {
	list->firstElement = NULL;
	list->activeElement = NULL;
	list->lastElement = NULL;
	list->currentLength = 0;
}

/**
 * Zruší všechny prvky seznamu list a uvede seznam do stavu, v jakém se nacházel
 * po inicializaci.
 * Rušené prvky seznamu budou korektně uvolněny voláním operace free.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Dispose( DLList *list ) {
	while (list->firstElement != NULL) {
		DLLElementPtr tmp_ptr = list->firstElement;
		list->firstElement = list->firstElement->nextElement;

		free(tmp_ptr);
		list->currentLength--;
	}
	list->activeElement = NULL;
	list->lastElement = NULL;
}

/**
 * Vloží nový prvek na začátek seznamu list.
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení na začátek seznamu
 */
void DLL_InsertFirst( DLList *list, long data ) {
	DLLElementPtr newElemPtr = (DLLElementPtr) malloc (sizeof(struct DLLElement));
	if (newElemPtr == NULL) {
		DLL_Error();
	}

	newElemPtr->data = data;
	newElemPtr->nextElement = list->firstElement;
	newElemPtr->previousElement = NULL;
	
	if (list->firstElement != NULL) { // if first elem exists
		list->firstElement->previousElement = newElemPtr;
	}
	else {
		list->lastElement = newElemPtr; // alpha and omega
	}

	list->firstElement = newElemPtr;
	list->currentLength++;
}

/**
 * Vloží nový prvek na konec seznamu list (symetrická operace k DLL_InsertFirst).
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení na konec seznamu
 */
void DLL_InsertLast( DLList *list, long data ) {
	DLLElementPtr newElemPtr = (DLLElementPtr) malloc (sizeof(struct DLLElement));
	if (newElemPtr == NULL) {
		DLL_Error();
		return;
	}
	newElemPtr->data = data;
	newElemPtr->nextElement = NULL; // no (r) neighbour exists 
	newElemPtr->previousElement = list->lastElement; // new last points to old last

	if (list->lastElement != NULL) { // old last points to old last
		list->lastElement->nextElement = newElemPtr;
	}
	else { // list was empty, so last = first
		list->firstElement = newElemPtr; // right neighbour points to new elem
	}
	list->lastElement = newElemPtr;
	list->currentLength++;
}

/**
 * Nastaví první prvek seznamu list jako aktivní.
 * Funkci implementujte jako jediný příkaz, aniž byste testovali,
 * zda je seznam list prázdný.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_First( DLList *list ) {
	list->activeElement = list->firstElement;
}

/**
 * Nastaví poslední prvek seznamu list jako aktivní.
 * Funkci implementujte jako jediný příkaz, aniž byste testovali,
 * zda je seznam list prázdný.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Last( DLList *list ) {
	list->activeElement = list->lastElement;
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu prvního prvku seznamu list.
 * Pokud je seznam list prázdný, volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void DLL_GetFirst( DLList *list, long *dataPtr ) {
	if (list->firstElement == NULL) {
		DLL_Error();
		return;
	}
	*dataPtr = list->firstElement->data;
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu posledního prvku seznamu list.
 * Pokud je seznam list prázdný, volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void DLL_GetLast( DLList *list, long *dataPtr ) {
	if (list->firstElement == NULL) {
		DLL_Error();
		return;
	}
	*dataPtr = list->lastElement->data;
}

/**
 * Zruší první prvek seznamu list.
 * Pokud byl první prvek aktivní, aktivita se ztrácí.
 * Pokud byl seznam list prázdný, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteFirst( DLList *list ) {
	DLLElementPtr tmp_ptr;
	if (list->firstElement != NULL) {
		tmp_ptr = list->firstElement;
		if (list->activeElement == list->firstElement) {
			list->activeElement = NULL;
		}
		if (list->firstElement == list->lastElement) {
			list->firstElement = NULL;
			list->lastElement = NULL;
		}
		else {
			list->firstElement = list->firstElement->nextElement;
			list->firstElement->previousElement = NULL;
		}
		free(tmp_ptr);
		list->currentLength--;
	} // list != empty
}

/**
 * Zruší poslední prvek seznamu list.
 * Pokud byl poslední prvek aktivní, aktivita seznamu se ztrácí.
 * Pokud byl seznam list prázdný, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteLast( DLList *list ) {
	if (list->firstElement != NULL) {
		if (list->activeElement == list->lastElement) {
			list->activeElement = NULL;
		}
		DLLElementPtr tmp_ptr = list->lastElement;
		if (list->firstElement == list->lastElement) {
			list->firstElement = NULL;
			list->lastElement = NULL;
		}
		else {
			tmp_ptr->previousElement->nextElement = NULL; // second last points to nothing
			list->lastElement = tmp_ptr->previousElement;
		}
		
		free(tmp_ptr);
		list->currentLength--; 
	} // list = empty
}

/**
 * Zruší prvek seznamu list za aktivním prvkem.
 * Pokud je seznam list neaktivní nebo pokud je aktivní prvek
 * posledním prvkem seznamu, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteAfter( DLList *list ) {
	if (list->activeElement != NULL) {
		if (list->activeElement != list->lastElement) {
			DLLElementPtr tmp_ptr;
			tmp_ptr = list->activeElement->nextElement;
			list->activeElement->nextElement = tmp_ptr->nextElement;
			if (tmp_ptr == list->lastElement) {
				list->lastElement = list->activeElement;
			}
			else {
				tmp_ptr->nextElement->previousElement = list->activeElement;
			}
			free(tmp_ptr);
			list->currentLength--;
		} // active != last
	} // list != inactive
}

/**
 * Zruší prvek před aktivním prvkem seznamu list .
 * Pokud je seznam list neaktivní nebo pokud je aktivní prvek
 * prvním prvkem seznamu, nic se neděje.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_DeleteBefore( DLList *list ) {
	if ((list->activeElement != NULL) && (list->firstElement != NULL)) {
		DLLElementPtr tmp_ptr = list->activeElement->previousElement;

		if (tmp_ptr != list->firstElement && tmp_ptr != NULL) { // active was first element
			list->activeElement->previousElement = tmp_ptr->previousElement; // (active->prev) = (delEl->prev)
			tmp_ptr->previousElement->nextElement = list->activeElement; // (delEl->prev->next) = (active) 
			free(tmp_ptr);
		}
		else if (tmp_ptr == list->firstElement ) { // if delElem is first
			list->firstElement = list->activeElement;
			list->firstElement->previousElement = NULL; // no elems before the first
			free(tmp_ptr);
		}
		list->currentLength--;
	} // list != empty && active
} 

/**
 * Vloží prvek za aktivní prvek seznamu list.
 * Pokud nebyl seznam list aktivní, nic se neděje.
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení do seznamu za právě aktivní prvek
 */
void DLL_InsertAfter( DLList *list, long data ) {
	if (list->activeElement != NULL) {
		DLLElementPtr newElemPtr = (DLLElementPtr) malloc (sizeof(struct DLLElement));
		if (newElemPtr == NULL) {
			DLL_Error();
			return;
		}
		newElemPtr->data = data;
		newElemPtr->nextElement = list->activeElement->nextElement; // set neighbouring elem (r) for the newEl
		newElemPtr->previousElement = list->activeElement; // set neighbouring elem (l), (insert after active)

		list->activeElement->nextElement =  newElemPtr; // active points to inserted elem

		if (list->activeElement == list->lastElement) { //inserting after the end, new end is new elem
			list->lastElement = newElemPtr;
		}
		else { // inserting somewhere inside the list
			newElemPtr->nextElement->previousElement = newElemPtr; // right neighbour points to new elem
		}
		list->currentLength++;
	} // list = active
}

/**
 * Vloží prvek před aktivní prvek seznamu list.
 * Pokud nebyl seznam list aktivní, nic se neděje.
 * V případě, že není dostatek paměti pro nový prvek při operaci malloc,
 * volá funkci DLL_Error().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Hodnota k vložení do seznamu před právě aktivní prvek
 */
void DLL_InsertBefore( DLList *list, long data ) {
	if (list->activeElement != NULL) {
		DLLElementPtr newElemPtr = (DLLElementPtr) malloc (sizeof(struct DLLElement));
		if (newElemPtr == NULL) {
			DLL_Error();
			return;
		}
		newElemPtr->data = data;

		if (list->activeElement != list->firstElement) {
			newElemPtr->nextElement = list->activeElement; // (newElem->next) = (active)
			newElemPtr->previousElement = list->activeElement->previousElement; // (newElem->prev) = (l)
			list->activeElement->previousElement->nextElement = newElemPtr; // (l->next) = (newElem)
			list->activeElement->previousElement = newElemPtr; // (active->prev) = (newElem)
		}
		else { // active was first
			newElemPtr->nextElement = list->firstElement;
			newElemPtr->previousElement = NULL;
			list->firstElement->previousElement = newElemPtr;
			list->firstElement = newElemPtr;
		}
		list->currentLength++;
	} // list = active
}

/**
 * Prostřednictvím parametru dataPtr vrátí hodnotu aktivního prvku seznamu list.
 * Pokud seznam list není aktivní, volá funkci DLL_Error ().
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param dataPtr Ukazatel na cílovou proměnnou
 */
void DLL_GetValue( DLList *list, long *dataPtr ) {
	if (list->activeElement == NULL) {
		DLL_Error();
		return;
	} // list != active
	*dataPtr = list->activeElement->data;
}

/**
 * Přepíše obsah aktivního prvku seznamu list.
 * Pokud seznam list není aktivní, nedělá nic.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 * @param data Nová hodnota právě aktivního prvku
 */
void DLL_SetValue( DLList *list, long data ) {
	if (list->activeElement != NULL) {
		list->activeElement->data = data;
	} // list = active
}

/**
 * Posune aktivitu na následující prvek seznamu list.
 * Není-li seznam aktivní, nedělá nic.
 * Všimněte si, že při aktivitě na posledním prvku se seznam stane neaktivním.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Next( DLList *list ) {
	if (list->activeElement != NULL) {
		list->activeElement = list->activeElement->nextElement;
	} // list = active
}


/**
 * Posune aktivitu na předchozí prvek seznamu list.
 * Není-li seznam aktivní, nedělá nic.
 * Všimněte si, že při aktivitě na prvním prvku se seznam stane neaktivním.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 */
void DLL_Previous( DLList *list ) {
	if (list->activeElement != NULL) {
		list->activeElement = list->activeElement->previousElement;
	} // list = active
}

/**
 * Je-li seznam list aktivní, vrací nenulovou hodnotu, jinak vrací 0.
 * Funkci je vhodné implementovat jedním příkazem return.
 *
 * @param list Ukazatel na inicializovanou strukturu dvousměrně vázaného seznamu
 *
 * @returns Nenulovou hodnotu v případě aktivity prvku seznamu, jinak nulu
 */
bool DLL_IsActive( DLList *list ) {
	return list->activeElement != NULL;
}

/* Konec c206.c */
