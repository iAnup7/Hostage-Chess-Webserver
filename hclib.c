#include <stdio.h>
#include <stdlib.h>
#include <ctype.h> 
#include <string.h>    
#include "hclib.h"

exboard_t *newboard(void){

    exboard_t *board = (exboard_t *)malloc(sizeof(exboard_t));
    if (board == NULL){
        printf("Error: malloc failed\n");
        exit(1);
    }

    // Initialize the airfilds and prisons
    for (int i = 0; i < 16; i++){
        board->bprison[i] = ' ';
        board->bairfield[i] = ' ';
        board->wprison[i] = ' ';
        board->wairfield[i] = ' ';
    }


    // Initialize the board
    for (int i = 0; i < 8; i++){
        for (int j = 0; j < 8; j++){
            board->board[i][j] = ' ';
        }
    }

    board->bprison[0] = '\0';
    board->bairfield[0] = '\0';
    board->wprison[0] = '\0';
    board->wairfield[0] = '\0';


    // // Initialize the white airfield
    // for (int i = 0; i < 16; i++){
    //     board->wairfield[i] = ' ';
    // }

    

    // Initialize the board
    board->board[0][0] = 'R';
    board->board[0][1] = 'N';
    board->board[0][2] = 'B';
    board->board[0][3] = 'Q';
    board->board[0][4] = 'K';
    board->board[0][5] = 'B';
    board->board[0][6] = 'N';
    board->board[0][7] = 'R';

    board->board[7][0] = 'r';
    board->board[7][1] = 'n';
    board->board[7][2] = 'b';
    board->board[7][3] = 'q';
    board->board[7][4] = 'k';
    board->board[7][5] = 'b';
    board->board[7][6] = 'n';
    board->board[7][7] = 'r';

    board->board[1][0] = 'P';
    board->board[1][1] = 'P';
    board->board[1][2] = 'P';
    board->board[1][3] = 'P';
    board->board[1][4] = 'P';
    board->board[1][5] = 'P';
    board->board[1][6] = 'P';
    board->board[1][7] = 'P';

    board->board[6][0] = 'p';
    board->board[6][1] = 'p';
    board->board[6][2] = 'p';
    board->board[6][3] = 'p';
    board->board[6][4] = 'p';
    board->board[6][5] = 'p';
    board->board[6][6] = 'p';
    board->board[6][7] = 'p';

    


    return board;

}

exboard_t *copyboard(exboard_t *board) {
    exboard_t *new_board = (exboard_t *)malloc(sizeof(exboard_t));
    if (new_board == NULL) {
        return NULL;
    }
    memcpy(new_board, board, sizeof(exboard_t)); // Copy the board
    return new_board;
}

char *stringboard(exboard_t *board) {
    char *str = (char *)malloc(162);
    if (str == NULL) {
        return NULL; // Handle malloc failure
    }

    // Initialize all to spaces
    for (int i = 0; i < 161; i++) {
        str[i] = ' ';
    }

    // Every 9th location is a newline
    for (int i = 8; i < 161; i += 9) {
        str[i] = '\n';
    }

    // Fill indices 36-43 with '-'
    for (int i = 36; i < 44; i++) {
        str[i] = '-';
    }

    // Fill indices 117-124 with '-'
    for (int i = 117; i < 125; i++) {
        str[i] = '-';
    }

    // Copy bprison
    for (int i = 0; i < 8; i++) {
        str[i] = board->bprison[i];
        str[i + 9] = board->bprison[i + 8];
    }

    // Copy bairfield
    for (int i = 0; i < 8; i++) {
        str[i + 18] = board->bairfield[i];
        str[i + 27] = board->bairfield[i + 8];
    }

    // Copy board rows to string
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            str[45 + i * 9 + j] = board->board[7 - i][j];
        }
    }

    // Copy wairfield
    for (int i = 0; i < 8; i++) {
        str[i + 126] = board->wairfield[i];
        str[i + 135] = board->wairfield[i + 8];
    }

    // Copy wprison
    for (int i = 0; i < 8; i++) {
        str[i + 144] = board->wprison[i];
        str[i + 153] = board->wprison[i + 8];
    }

    // Set the last character to the null terminator
    str[161] = '\0';

    return str;
}


// The apply_move function applies a move to the board. It moves a piece from one square to another and handles captures and promotions.
exboard_t *apply_move( exboard_t *board, move_t *move ) {
    // Get the piece at the destination square
    char captured_piece = board->board[move->to_i][move->to_j];
    if (captured_piece != ' ') {
         // If the captured piece is a lowercase letter, it is a black piece and is added to the white prison
        if (captured_piece >= 'a' && captured_piece <= 'z') {
            int i = 0;
            while (board->wprison[i] != '\0') i++;
            board->wprison[i] = captured_piece;
        } else {
            // If the captured piece is an uppercase letter, it is a white piece and is added to the black prison
            int i = 0;
            while (board->bprison[i] != '\0') i++;
            board->bprison[i] = captured_piece;
        }
    }
    // If the move is a promotion, the piece at the destination square is replaced with the promotion piece
    if (move->promotion != ' ') {
        board->board[move->to_i][move->to_j] = move->promotion;
    } else {
        // If the move is not a promotion, the piece at the destination square is replaced with the piece from the source square
        board->board[move->to_i][move->to_j] = board->board[move->from_i][move->from_j];
    }// The source square is now empty
    board->board[move->from_i][move->from_j] = ' ';
    return board;
}
// The moves function returns a list of all legal moves for a piece at a given square
move_t **moves(board_t *board, int from_i, int from_j) {
    char piece = (*board)[from_i][from_j];
    int colour = piece >= 'a' && piece <= 'z' ? 0 : 1; // 0 for white, 1 for black
    // Return the appropriate list of moves based on the type of piece
    switch(tolower(piece)) {
        case 'q':
            return queenmoves(board, from_i, from_j, colour);
        case 'p':
            return pawn_moves(board, from_i, from_j, colour);
        case 'b':
            return bishopmoves(board, from_i, from_j, colour);
        case 'n':
            return knightmoves(board, from_i, from_j, colour);
        case 'k':
            return king_moves(board, from_i, from_j, colour);
        case 'r':
            return rookmoves(board, from_i, from_j, colour);
        default:
            printf("Error: Invalid piece\n");
            exit(1);
    }
}

move_t **knightmoves(board_t *board, int from_i, int from_j, int colour) {
    // The 8 possible knight moves
    int directions[8][2] = {
        {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
        {1, -2}, {1, 2}, {2, -1}, {2, 1}
    };
    
    // Allocate memory for up to 9 moves (8 moves + NULL terminator)
    move_t **moves = (move_t **)malloc(9 * sizeof(move_t *));
    int move_count = 0; // Track the number of valid moves

    // Loop through all possible knight moves
    for (int i = 0; i < 8; i++) {
        int new_i = from_i + directions[i][0];
        int new_j = from_j + directions[i][1];

        // Ensure the new position is within bounds of the board
        if (new_i >= 0 && new_i < 8 && new_j >= 0 && new_j < 8) {
            char target = (*board)[new_i][new_j];

            // Check if the target is empty or contains an opponent's piece
            if (target == ' ' || 
                (colour == 0 && target >= 'a' && target <= 'z') || // Opponent is black
                (colour == 1 && target >= 'A' && target <= 'Z')) { // Opponent is white

                // Allocate and populate the move
                moves[move_count] = (move_t *)malloc(sizeof(move_t));
                moves[move_count]->from_i = from_i;
                moves[move_count]->from_j = from_j;
                moves[move_count]->to_i = new_i;
                moves[move_count]->to_j = new_j;
                moves[move_count]->promotion = ' ';
                moves[move_count]->hostage = target; // The captured piece (if any)

                move_count++;
            }
        }
    }

    // Terminate the move array with NULL
    moves[move_count] = NULL;
    
    return moves;  // Return the list of moves
}

// // The knightmoves function generates all legal moves for a knight at a given square
// move_t **knightmoves(board_t *board, int from_i, int from_j, int colour) {
    
//      // The directions array represents the 8 possible moves for a knight
//     int directions[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};
//     int test_count = 0;
//     int i = 0;
//     move_t **moves = (move_t **)malloc(9 * sizeof(move_t *));
//     // Loop through each possible direction
//     for (int j = 0; j < 8; j++) {
//         int new_i = from_i + directions[j][0];
//         int new_j = from_j + directions[j][1];
//         if (new_i >= 0 && new_i < 8 && new_j >= 0 && new_j < 8 && 
//             ((*board)[new_i][new_j] == ' ' || 
//             (colour == 0 && (*board)[new_i][new_j] >= 'a' && (*board)[new_i][new_j] <= 'z') || 
//             (colour == 1 && (*board)[new_i][new_j] >= 'A' && (*board)[new_i][new_j] <= 'Z'))){
//             moves[i] = (move_t *)malloc(sizeof(move_t));
//             moves[i]->promotion = ' ';
//             test_count = 1 + test_count;
//             moves[i]->to_j = new_j;
//             moves[i]->from_j = from_j;
//             moves[i]->to_i = new_i;
//             test_count++;
//             moves[i]->from_i = from_i;
//             moves[i]->hostage = (*board)[new_i][new_j];
            
            
//             i++;
//         }
//     }
//     // Set the last element of the moves array to NULL
//     moves[i] = NULL;
//     // Reallocate the moves array to the correct size
//     moves = (move_t **)realloc(moves, (i+1) * sizeof(move_t *));
//     return moves;
// }
// The knightmoves function generates all legal moves for a bishop at a given square

move_t **bishopmoves(board_t *board, int from_i, int from_j, int colour) {
    // Allocate memory for the moves list (we'll realloc later if needed)
    move_t **moves = (move_t **)malloc(32 * sizeof(move_t *)); // Bishop can theoretically have up to 14 moves
    int move_count = 0; // Track number of valid moves

    // Check if malloc failed
    if (moves == NULL) {
        printf("Error: malloc failed\n");
        exit(1);
    }

    // Possible directions a bishop can move (diagonally in 4 directions)
    int directions[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

    // Loop over all 4 diagonal directions
    for (int j = 0; j < 4; j++) {
        // Move outward in the current direction until hitting the board edge or a piece
        for (int distance = 1; distance < 8; distance++) {
            int new_i = from_i + directions[j][0] * distance;
            int new_j = from_j + directions[j][1] * distance;

            // Check if the new square is still within the bounds of the board
            if (new_i >= 0 && new_i < 8 && new_j >= 0 && new_j < 8) {
                char target_piece = (*board)[new_i][new_j];

                // If the square is empty, it's a valid move
                if (target_piece == ' ') {
                    moves[move_count] = (move_t *)malloc(sizeof(move_t));
                    moves[move_count]->from_i = from_i;
                    moves[move_count]->from_j = from_j;
                    moves[move_count]->to_i = new_i;
                    moves[move_count]->to_j = new_j;
                    moves[move_count]->promotion = ' ';
                    moves[move_count]->hostage = target_piece;
                    move_count++;
                }
                // If the square contains an opponent's piece, it's a valid capture move, but we must stop afterward
                else if ((colour == 0 && target_piece >= 'a' && target_piece <= 'z') || // white bishop, black piece
                         (colour == 1 && target_piece >= 'A' && target_piece <= 'Z')) {  // black bishop, white piece
                    moves[move_count] = (move_t *)malloc(sizeof(move_t));
                    moves[move_count]->from_i = from_i;
                    moves[move_count]->from_j = from_j;
                    moves[move_count]->to_i = new_i;
                    moves[move_count]->to_j = new_j;
                    moves[move_count]->promotion = ' ';
                    moves[move_count]->hostage = target_piece;
                    move_count++;
                    break;  // Stop after capturing a piece
                }
                // If the square contains a friendly piece, stop in this direction
                else {
                    break;
                }
            } else {
                // Stop if the new position is outside the board
                break;
            }
        }
    }

    // Terminate the moves array with NULL
    moves[move_count] = NULL;

    // Optionally realloc the moves array to the correct size (if necessary)
    moves = (move_t **)realloc(moves, (move_count + 1) * sizeof(move_t *));
    if (moves == NULL) {
         printf("Error: realloc failed\n");
         exit(1);
     }
    
    return moves;  // Return the list of valid moves
}

// The directions array represents the 4 possible directions for a rook
move_t **rookmoves( board_t *board, int from_i, int from_j, int colour ){
    move_t **moves = (move_t **)malloc(32 * sizeof(move_t *));
    if (moves == NULL){
        printf("Error: malloc failed\n");
        exit(1);
    }

    for (int i = 0; i < 32; i++){
        moves[i] = (move_t *)malloc(sizeof(move_t));
        if (moves[i] == NULL){
            printf("Error: malloc failed\n");
            exit(1);
        }
    }

    int i = 0;
    // The directions array represents the 4 possible directions for a bishop
    int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    // Loop through each square in the direction until the edge of the board
    for (int j = 0; j < 4; j++) {
        for (int distance = 1; distance < 8; distance++) {

            int new_j = directions[j][1] * distance + from_j;
            int new_i = from_i + directions[j][0] * distance;
            
            if (new_i >= 0 && new_i < 8 && new_j >= 0 && new_j < 8) {
                if ((*board)[new_i][new_j] == ' ' || 
                    (colour == 0 && (*board)[new_i][new_j] >= 'a' && (*board)[new_i][new_j] <= 'z') ||
                    (colour == 1 && (*board)[new_i][new_j] >= 'A' && (*board)[new_i][new_j] <= 'Z')){
                    // Set the source and destination squares for the move
                    moves[i]->from_i = from_i;
                    moves[i]->from_j = from_j;
                    moves[i]->to_i = new_i;
                    moves[i]->to_j = new_j;
                    // Set the promotion and captured piece for the move
                    moves[i]->promotion = ' ';
                    moves[i]->hostage = (*board)[new_i][new_j];
                    i++;
                }
                // If the new square is not empty, break the loop
                if ((*board)[new_i][new_j] != ' ') {
                    break;
                }
            } else {
                break;
            }
        }
    }
     // Set the last element of the moves array to NULL
    moves[i] = NULL;
     // Reallocate the moves array to the correct size
    moves = (move_t **)realloc(moves, (i+1) * sizeof(move_t *));
    return moves;
}


// The queenmoves function generates all legal moves for a king at a given square
move_t **king_moves( board_t *board, int from_i, int from_j, int colour ){
    move_t **moves = (move_t **)malloc(9 * sizeof(move_t *));
    if (moves == NULL){
        printf("Error: malloc failed\n");
        exit(1);
    }

    for (int i = 0; i < 9; i++){
        moves[i] = (move_t *)malloc(sizeof(move_t));
        if (moves[i] == NULL){
            printf("Error: malloc failed\n");
            exit(1);
        }
    }

    int i = 0;
    int directions[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
    for (int j = 0; j < 8; j++) {
        int new_j = from_j + directions[j][1];
        int new_i = from_i + directions[j][0];
        
        if (new_i >= 0 && new_i < 8 && new_j >= 0 && new_j < 8 && 
            ((*board)[new_i][new_j] == ' ' || 
            (colour == 0 && (*board)[new_i][new_j] >= 'a' && (*board)[new_i][new_j] <= 'z') || 
            (colour == 1 && (*board)[new_i][new_j] >= 'A' && (*board)[new_i][new_j] <= 'Z'))){
            moves[i]->from_i = from_i;
            moves[i]->from_j = from_j;
            moves[i]->to_i = new_i;
            moves[i]->to_j = new_j;
            moves[i]->promotion = ' ';
            moves[i]->hostage = (*board)[new_i][new_j];
            i++;
        }
    }

    moves[i] = NULL;
    moves = (move_t **)realloc(moves, (i+1) * sizeof(move_t *));
    return moves;
}
// The queenmoves function generates all legal moves for a queen at a given square
move_t **queenmoves(board_t *board, int from_i, int from_j, int colour) {
    // Generate all legal rook moves for the queen
    move_t **rook_moves = rookmoves(board, from_i, from_j, colour);
    int total_moves = 0;
    int test_count = 0;
    // Generate all legal bishop moves for the queen
    move_t **bishop_moves = bishopmoves(board, from_i, from_j, colour);
    // Count the number of rook moves
    int rook_moves_count = 0;
    while (rook_moves[rook_moves_count] != NULL) {
        rook_moves_count++;
        total_moves++;
    }
    // Count the number of bishop moves
    int bishop_moves_count = 0;
    while (bishop_moves[bishop_moves_count] != NULL) {
        total_moves++;
        bishop_moves_count++;
        
    }

    test_count = 1 + test_count;    
    test_count = total_moves;
    move_t **moves = (move_t **)malloc((bishop_moves_count + rook_moves_count + 1) * sizeof(move_t *));
    // Copy the rook moves to the moves array
    
        // Copy the rook moves to the moves array
    memcpy(moves, rook_moves, rook_moves_count * sizeof(move_t *));
    // Copy the bishop moves to the moves array
    memcpy(moves + rook_moves_count, bishop_moves, bishop_moves_count * sizeof(move_t *));
    moves[bishop_moves_count + rook_moves_count] = NULL;
    // Free the memory allocated for the rook and bishop moves
    free(bishop_moves);
    free(rook_moves);

    return moves;
}

move_t **pawn_moves(board_t *board, int from_i, int from_j, int colour) {
    move_t **moves = (move_t **)malloc(5 * sizeof(move_t *));
    if (moves == NULL) {
        printf("Error: malloc failed\n");
        exit(1);
    }

    int direction = (colour == 0) ? -1 : 1;  // 0 for white (up the board), 1 for black (down the board)
    int i = 0;
    int test_count = 0;

    // Move one square forward if it's empty
    if ((*board)[from_i + direction][from_j] == ' ') {
        moves[i] = (move_t *)malloc(sizeof(move_t));
        if (moves[i] == NULL) {
            printf("Error: malloc failed\n");
            exit(1);
        }
        moves[i]->from_i = from_i;
        moves[i]->from_j = from_j;
        moves[i]->to_i = from_i + direction;
        moves[i]->to_j = from_j;
        moves[i]->promotion = ' ';
        moves[i]->hostage = ' ';
        test_count = 1 + test_count;
        i++;
    }

    // Capture diagonally to the right
    if (from_j + 1 < 8 && 
        ((colour == 0 && (*board)[from_i + direction][from_j + 1] >= 'a' && (*board)[from_i + direction][from_j + 1] <= 'z') ||
         (colour == 1 && (*board)[from_i + direction][from_j + 1] >= 'A' && (*board)[from_i + direction][from_j + 1] <= 'Z'))) {
        moves[i] = (move_t *)malloc(sizeof(move_t));
        if (moves[i] == NULL) {
            printf("Error: malloc failed\n");
            exit(1);
        }
        moves[i]->from_i = from_i;
        moves[i]->from_j = from_j;
        moves[i]->to_i = from_i + direction;
        moves[i]->to_j = from_j + 1;
        moves[i]->promotion = ' ';
        moves[i]->hostage = (*board)[from_i + direction][from_j + 1];
        test_count = 1 + test_count;
        i++;
    }

    // Capture diagonally to the left
    if (from_j - 1 >= 0 && 
        ((colour == 0 && (*board)[from_i + direction][from_j - 1] >= 'a' && (*board)[from_i + direction][from_j - 1] <= 'z') ||
         (colour == 1 && (*board)[from_i + direction][from_j - 1] >= 'A' && (*board)[from_i + direction][from_j - 1] <= 'Z'))) {
        moves[i] = (move_t *)malloc(sizeof(move_t));
        if (moves[i] == NULL) {
            printf("Error: malloc failed\n");
            exit(1);
        }
        moves[i]->from_i = from_i;
        moves[i]->from_j = from_j;
        moves[i]->to_i = from_i + direction;
        moves[i]->to_j = from_j - 1;
        moves[i]->promotion = ' ';
        moves[i]->hostage = (*board)[from_i + direction][from_j - 1];
        i++;
    }

    // Move two squares forward from the starting position
    if ((colour == 0 && from_i == 6 && (*board)[from_i - 1][from_j] == ' ' && (*board)[from_i - 2][from_j] == ' ') ||
        (colour == 1 && from_i == 1 && (*board)[from_i + 1][from_j] == ' ' && (*board)[from_i + 2][from_j] == ' ')) {
        moves[i] = (move_t *)malloc(sizeof(move_t));
        if (moves[i] == NULL) {
            printf("Error: malloc failed\n");
            exit(1);
        }
        moves[i]->from_i = from_i;
        moves[i]->from_j = from_j;
        moves[i]->to_i = from_i + 2 * direction;
        moves[i]->to_j = from_j;
        moves[i]->promotion = ' ';
        moves[i]->hostage = ' ';
        i++;
    }

    // Set the last element of the moves array to NULL
    moves[i] = NULL;

    // Reallocate the moves array to the correct size
    moves = (move_t **)realloc(moves, (i + 1) * sizeof(move_t *));
    if (moves == NULL) {
        printf("Error: realloc failed\n");
        exit(1);
    }

    return moves;
}



char *fen(exboard_t *board, char *active, char *castling, char *enpassant, int half, int full) {
    int len = 0;
    int empty_count = 0;

    // Calculate the length of the board representation in FEN (traverse reversed)
    for (int i = 7; i >= 0; i--) { // Reverse the row iteration
        empty_count = 0;
        for (int j = 7; j >= 0; j--) { // Reverse the column iteration
            if (board->board[i][j] == ' ') {
                empty_count++;
            } else {
                if (empty_count > 0) {
                    len += snprintf(NULL, 0, "%d", empty_count);
                    empty_count = 0;
                }
                len++;
            }
        }
        if (empty_count > 0) {
            len += snprintf(NULL, 0, "%d", empty_count);
        }
        if (i > 0) { // Add '/' except for the last row
            len++;
        }
    }

    // Add lengths of additional strings and spaces
    len += strlen(active) + 1;
    len += strlen(castling) + 1;
    len += strlen(enpassant) + 1;
    len += snprintf(NULL, 0, "%d", half) + 1;
    len += snprintf(NULL, 0, "%d", full) + 1;

    // Allocate memory for the FEN string
    char *fen_str = (char *)malloc(len + 1);
    //Piece of code updated to reverse the traversal
    // Constructing the reversed FEN string
    int pos = 0;
    for (int i = 7; i >= 0; i--) { // Reversed the row traversal
        empty_count = 0;
        for (int j = 7; j >= 0; j--) { // Reversed the column traversal
            if (board->board[i][j] == ' ') {
                empty_count++;
            } else {
                if (empty_count > 0) {
                    pos += sprintf(&fen_str[pos], "%d", empty_count);
                    empty_count = 0;
                }
                fen_str[pos++] = board->board[i][j];
            }
        }
        if (empty_count > 0) {
            pos += sprintf(&fen_str[pos], "%d", empty_count);
        }
        if (i > 0) { // Add '/' except for the last row
            fen_str[pos++] = '/';
        }
    }

    // Append additional strings
    pos += sprintf(&fen_str[pos], " %s %s %s %d %d", active, castling, enpassant, half, full);

    // Null-terminate the string
    fen_str[pos] = '\0';

    return fen_str;
}

exboard_t *boardstring(char *string) {
    exboard_t *board = (exboard_t *)malloc(sizeof(exboard_t));

    if (board == NULL) {
        printf("Error: Memory allocation failed for exboard_t.\n");
        return NULL;
    }

    // Initialize all fields to spaces
    memset(board->bprison, ' ', 16);
    memset(board->bairfield, ' ', 16);
    memset(board->board, ' ', 64);
    memset(board->wairfield, ' ', 16);
    memset(board->wprison, ' ', 16);

    // Parse wprison
    for (int i = 0; i < 8; i++) {
        board->wprison[i] = string[i + 144];
        board->wprison[i + 8] = string[i + 153];
    }

    // Parse bprison
    for (int i = 0; i < 8; i++) {
        board->bprison[i] = string[i];
        board->bprison[i + 8] = string[i + 9];

         // Parse wairfield
    for (int i = 0; i < 8; i++) {
        board->wairfield[i] = string[i + 126];
        board->wairfield[i + 8] = string[i + 135];
    }
    }

    // Parse bairfield
    for (int i = 0; i < 8; i++) {
        board->bairfield[i] = string[i + 18];
        board->bairfield[i + 8] = string[i + 27];
    }

    // Parse board
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            board->board[7 - i][j] = string[45 + i * 9 + j];
        }
    }

   

    

    return board;
}
