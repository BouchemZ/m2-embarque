UTILISATION

cd 2526.given/arm.boot
make clean
make run

TOP line devrait contenir un Timer et un conteur d'evenements mais problème avec la FIFO Tx donc retirer car sinon il y a perte de byte.
la topline peut être décommenté dans isr.c.

Il y a un draft pour l'utilisation de stream pour de l'évenementiel mais je n'ai pas réussi à le faire marcher.
