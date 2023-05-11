int atoi(const char *str) {
    int result = 0;
    int sign = 1;
    int i = 0;

    // Ignorar espacios iniciales
    while (str[i] == ' ')
        i++;

    // Comprobar signo
    if (str[i] == '-') {
        sign = -1;
        i++;
    } else if (str[i] == '+') {
        i++;
    }

    // Convertir dígitos a entero
    while (str[i] >= '0' && str[i] <= '9') {
        result = result * 10 + (str[i] - '0');
        i++;
    }

    return result * sign;
}

