#include <libstr.h>
#include <stddef.h>

/**
 * Calcula la longitud de una cadena.
 * @param str La cadena de entrada.
 * @return El número de caracteres en la cadena (sin incluir el terminador nulo).
 */
size_t k_strlen(const char* str) {
    const char* p = str;
    while (*p != '\0') {
        ++p;
    }
    return p - str;
}

/**
 * Copia una cadena de origen en una cadena de destino.
 * @param dest La cadena de destino.
 * @param src La cadena de origen.
 */
void k_strcpy(char* dest, const char* src) {
    while ((*dest++ = *src++) != '\0') {}
}

/**
 * Copia los primeros 'n' caracteres de una cadena de origen en una cadena de destino.
 * Si la cadena de origen es más corta que 'n', se rellena la cadena de destino con terminadores nulos adicionales.
 * @param dest La cadena de destino.
 * @param src La cadena de origen.
 * @param n El número máximo de caracteres a copiar.
 */
void k_strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; ++i) {
        dest[i] = src[i];
    }
    for (; i < n; ++i) {
        dest[i] = '\0';
    }
}

/**
 * Concatena una cadena de origen al final de una cadena de destino.
 * @param dest La cadena de destino.
 * @param src La cadena de origen.
 */
void k_strcat(char* dest, const char* src) {
    size_t dest_len = k_strlen(dest);
    while (*src != '\0') {
        dest[dest_len++] = *src++;
    }
    dest[dest_len] = '\0';
}

/**
 * Concatena los primeros 'n' caracteres de una cadena de origen al final de una cadena de destino.
 * Si la cadena de origen es más corta que 'n', se rellena la cadena de destino con un terminador nulo adicional.
 * @param dest La cadena de destino.
 * @param src La cadena de origen.
 * @param n El número máximo de caracteres a concatenar.
 */
void k_strncat(char* dest, const char* src, size_t n) {
    size_t dest_len = k_strlen(dest);
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; ++i) {
        dest[dest_len + i] = src[i];
    }
    dest[dest_len + i] = '\0';
}

/**
 * Compara dos cadenas lexicográficamente.
 * @param s1 La primera cadena a comparar.
 * @param s2 La segunda cadena a comparar.
 * @return Un valor negativo si s1 es menor que s2, un valor positivo si s1 es mayor que s2, o 0 si son iguales.
 */
int k_strcmp(const char* s1, const char* s2) {
    while (*s1 != '\0' && *s1 == *s2) {
        ++s1;
        ++s2;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

/**
 * Copia un bloque de memoria de origen a un bloque de memoria de destino.
 * @param dest El bloque de memoria de destino.
 * @param src El bloque de memoria de origen.
 * @param n El número de bytes a copiar.
 * @return Un puntero al bloque de memoria de destino.
 */
void* k_memcpy(void* dest, const void* src, size_t n) {
    unsigned char* cdest = (unsigned char*)dest;
    const unsigned char* csrc = (const unsigned char*)src;
    size_t i;
    for (i = 0; i < n; ++i) {
        cdest[i] = csrc[i];
    }
    return dest;
}

/**
 * Establece un bloque de memoria con un valor específico.
 * @param ptr El puntero al bloque de memoria.
 * @param value El valor a establecer.
 * @param num El número de bytes a establecer.
 */
void k_memset(void* ptr, int value, size_t num) {
    unsigned char* p = (unsigned char*)ptr;
    unsigned char val = (unsigned char)value;
    size_t i;
    for (i = 0; i < num; ++i) {
        p[i] = val;
    }
}

/**
 * Busca la primera aparición de un carácter en una cadena.
 * @param s La cadena de entrada.
 * @param c El carácter a buscar.
 * @return Un puntero a la primera aparición de 'c' en 's', o NULL si no se encuentra.
 */
char* k_strchr(const char* s, int c) {
    while (*s != '\0') {
        if (*s == c) {
            return (char*)s;
        }
        ++s;
    }
    return NULL;
}

/**
 * Calcula la longitud del segmento inicial de una cadena que contiene solo caracteres presentes en otra cadena.
 * @param s La cadena de entrada.
 * @param accept La cadena de caracteres válidos.
 * @return El número de caracteres consecutivos en 's' que están presentes en 'accept'.
 */
size_t k_strspn(const char* s, const char* accept) {
    size_t count = 0;
    const char* p;
    while (*s != '\0') {
        for (p = accept; *p != '\0'; ++p) {
            if (*s == *p) {
                break;
            }
        }
        if (*p == '\0') {
            return count;
        }
        ++count;
        ++s;
    }
    return count;
}

/**
 * Busca la primera aparición de cualquier carácter en una cadena en otra cadena.
 * @param s La cadena de entrada.
 * @param accept La cadena de caracteres a buscar.
 * @return Un puntero a la primera aparición de cualquier carácter de 'accept' en 's', o NULL si no se encuentra.
 */
char* k_strpbrk(const char* s, const char* accept) {
    const char* p;
    while (*s != '\0') {
        for (p = accept; *p != '\0'; ++p) {
            if (*s == *p) {
                return (char*)s;
            }
        }
        ++s;
    }
    return NULL;
}

/**
 * Divide una cadena en tokens utilizando un conjunto de delimitadores.
 * @param str La cadena de entrada.
 * @param delim Los delimitadores.
 * @return Un puntero al siguiente token encontrado en 'str', o NULL si no hay más tokens.
 *         El puntero se puede utilizar en llamadas posteriores para continuar la tokenización.
 */
char* k_strtok(char* str, const char* delim) {
    static char* last_token = NULL;
    char* token = NULL;

    if (str != NULL) {
        last_token = str;
    } else {
        if (last_token == NULL) {
            return NULL;
        }
        str = last_token;
    }

    str += k_strspn(str, delim);
    if (*str == '\0') {
        last_token = NULL;
        return NULL;
    }

    token = str;
    str = k_strpbrk(token, delim);
    if (str == NULL) {
        last_token = NULL;
    } else {
        *str = '\0';
        last_token = str + 1;
    }

    return token;
}
