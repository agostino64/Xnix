#include <libstr.h>

size_t k_strlen(const char *str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

void k_strcpy(char *dest, const char *src) {
    size_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

void k_strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[i] = src[i];
    }
    for ( ; i < n; i++) {
        dest[i] = '\0';
    }
}

void k_strcat(char *dest, const char *src) {
    size_t dest_len = k_strlen(dest);
    size_t i;
    for (i = 0; src[i] != '\0'; i++) {
        dest[dest_len + i] = src[i];
    }
    dest[dest_len + i] = '\0';
}

void k_strncat(char *dest, const char *src, size_t n) {
    size_t dest_len = k_strlen(dest);
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dest[dest_len + i] = src[i];
    }
    dest[dest_len + i] = '\0';
}

int k_strcmp(const char *s1, const char *s2) {
    size_t i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0') {
            return 0;
        }
    }
    return ((unsigned char) s1[i] < (unsigned char) s2[i]) ? -1 : 1;
}

void *k_memcpy(void *dest, const void *src, size_t n) {
    char *cdest = (char *) dest;
    const char *csrc = (const char *) src;
    size_t i;
    for (i = 0; i < n; i++) {
        cdest[i] = csrc[i];
    }
    return dest;
}

void k_memset(void *ptr, int value, size_t num) {
    unsigned char *p = ptr;
    unsigned char val = (unsigned char) value;
    size_t i;
    for (i = 0; i < num; i++) {
        p[i] = val;
    }
}

char *k_strchr(const char *s, int c) {
    while (*s != (char) c) {
        if (!*s++) {
            return NULL;
        }
    }
    return (char *) s;
}

size_t k_strspn(const char *s, const char *accept) {
    size_t count = 0;

    while (*s && k_strchr(accept, *s++)) {
        count++;
    }

    return count;
}

char *k_strpbrk(const char *s, const char *accept) {
    const char *p = s;

    while (*p) {
        if (k_strchr(accept, *p)) {
            return (char *) p;
        }
        p++;
    }

    return NULL;
}

char *k_strtok(char *str, const char *delim) {
    static char *last_token = NULL;
    char *token = NULL;

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
