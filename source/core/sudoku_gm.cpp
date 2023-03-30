#include <cassert>
#include <stdexcept>
#include <iterator>
#include <string.h>
#include <iostream>
#include <string.h>
#include <cctype>

#include "sudoku_gm.h"


namespace sdkg {

    /// Lambda expression that transform the string to lowercase.
    auto STR_LOWERCASE = [](const char * t)->std::string{
        std::string str{ t };
        std::transform( str.begin(), str.end(), str.begin(), ::tolower );
        return str;
    };

    /// Default constructor
    SudokuGame::SudokuGame() {
        m_opt.total_checks = 3; // Default value.
        m_opt.input_filename = "./data/input.txt"; // Default value.
    }

    void SudokuGame::usage( std::string msg ) const {
        if ( msg != "" ) std::cerr << "sudoku: " << msg << "\n\n";

        std::cerr << "Usage: sudoku [-c <num>] [--help] <input_puzzle_file>\n"
                  << "  Game options:\n"
                  << "    -c     <num> Number of checks per game. Default = 3.\n"
                  << "    --help       Print this help text.\n";
        std::cerr << std::endl;

        exit( msg != "" ? 1 : 0 );
    }

    void SudokuGame::process_events(){
        m_curr_msg = {""};
        if ( m_game_state == game_state_e::STARTING or
             m_game_state == game_state_e::HELPING  or
             m_game_state == game_state_e::CHECKING_MOVES or
             m_game_state == game_state_e::FINISHED_PUZZLE ) {
            // Reading a simple enter from user.
            std::string line;
            std::getline(std::cin, line);
        }
        else if ( m_game_state == game_state_e::READING_MAIN_OPT ) {

            std::string menu_op_in;
            std::getline(std::cin, menu_op_in);

            if      (menu_op_in == "1") m_curr_main_menu_opt = main_menu_opt_e::PLAY;
            else if (menu_op_in == "2") m_curr_main_menu_opt = main_menu_opt_e::NEW_GAME;
            else if (menu_op_in == "3") m_curr_main_menu_opt = main_menu_opt_e::QUIT;  
            else if (menu_op_in == "4") m_curr_main_menu_opt = main_menu_opt_e::HELP;

            menu_op_in.clear();
        }
        else if (m_game_state == game_state_e::PLAYING_MODE) {

            if (not board_isfull()) {
                game_play();
            } 
            else {

                game_result();
                /// atualizar infos de print
                game_end = true;

            }
            

        } 
        else if (m_game_state == game_state_e::REQUESTING_NEW_GAME) {
            new_board();
            
        }
        else if ( m_game_state == game_state_e::QUITTING ) {

            m_curr_msg = {"Are you sure you want to exit the game? [Y/N]"};
        }

        
    }

    void SudokuGame::update(){
        if (m_game_state == game_state_e::STARTING) {
            
            m_game_state = game_state_e::READING_MAIN_OPT;
        } 
        else if (m_game_state == game_state_e::READING_MAIN_OPT) {

            if      (m_curr_main_menu_opt == main_menu_opt_e::PLAY)
                m_game_state = game_state_e::PLAYING_MODE;
        
            else if (m_curr_main_menu_opt == main_menu_opt_e::NEW_GAME)
                m_game_state = game_state_e::REQUESTING_NEW_GAME;

            else if (m_curr_main_menu_opt == main_menu_opt_e::QUIT)
                m_game_state = game_state_e::QUITTING; 

            else if (m_curr_main_menu_opt == main_menu_opt_e::HELP) 
                m_game_state = game_state_e::HELPING;

        }
        else if ( m_game_state == game_state_e::PLAYING_MODE ) {

            if (not game_end) {
                if (checking) {
                    m_game_state = game_state_e::CHECKING_MOVES;
                }
                else {
                    m_game_state = game_state_e::PLAYING_MODE;                    
                }
            }
            else{
                m_game_state = game_state_e::FINISHED_PUZZLE;
            }
            
            
            
        }
        else if (m_game_state == game_state_e::REQUESTING_NEW_GAME) {
            

            m_curr_main_menu_opt = main_menu_opt_e::RESET;
            m_game_state = game_state_e::READING_MAIN_OPT;
        }
        else if (m_game_state == game_state_e::QUITTING) {
            
        
            m_game_state = game_state_e::CONFIRMING_QUITTING_MATCH;

        }
        else if (m_game_state == game_state_e::HELPING) {

            m_game_state = game_state_e::READING_MAIN_OPT;
        } else if (m_game_state == game_state_e::CONFIRMING_QUITTING_MATCH) {

            std::string confirm_quit;
            std::getline(std::cin, confirm_quit);
            if (confirm_quit == "y" or confirm_quit == "Y" or confirm_quit == "s" or confirm_quit == "S") {
                
                m_quitting_match = true;
            }
            if (m_quitting_match == true) {
                exit(0);
            } else {
                m_game_state = game_state_e::READING_MAIN_OPT;
            }
        }
        else if (m_game_state == game_state_e::CHECKING_MOVES) {
            
            
            if (checking) {
                checking = false;
                m_game_state = game_state_e::CHECKING_MOVES;
            }
            else {
                checking = false;
                m_game_state = game_state_e::PLAYING_MODE;
            }
            
            
        }
      
        
        
    }

    void SudokuGame::render(void) const{
        if (m_game_state == game_state_e::READING_MAIN_OPT) {
            title_render("MAIN SCREEN");
            board_render(game_actions[0]);
            msg_render(m_curr_msg);
            menu_render();
        } 
        else if (m_game_state == game_state_e::PLAYING_MODE) {
            title_render("PLAYING");
            board_render(game_actions.back());
            std::string checks;
            checks = std::to_string(m_checks_left);
            std::cout << Color::tcolor(">>> Checks left [ " + checks, Color::BRIGHT_YELLOW);
            std::cout << Color::tcolor(" ]\n",Color::BRIGHT_YELLOW);
            std::cout << Color::tcolor(">>> Digits left [ " + m_digits_left, Color::BRIGHT_YELLOW);
            std::cout << Color::tcolor(" ]\n", Color::BRIGHT_YELLOW);
            msg_render(m_curr_msg);
            display_play_cmd();
        } 
        else if (m_game_state == game_state_e::CHECKING_MOVES) {

            title_render("PLAYING");
            display_check(game_actions.back());
            std::string checks;
            checks = std::to_string(m_checks_left);
            std::cout << Color::tcolor(">>> Checks left [ " + checks, Color::BRIGHT_YELLOW);
            std::cout << Color::tcolor(" ]\n",Color::BRIGHT_YELLOW);
            std::cout << Color::tcolor(">>> Digits left [ " + m_digits_left, Color::BRIGHT_YELLOW);
            std::cout << Color::tcolor(" ]\n", Color::BRIGHT_YELLOW);
            msg_render(m_curr_msg);
            display_play_cmd();

        }
        else if (m_game_state == game_state_e::HELPING) {

            display_help();
        }
        else if (m_game_state == game_state_e::CONFIRMING_QUITTING_MATCH) {
            title_render("MAIN SCREEN");
            board_render(game_actions.back());
            msg_render(m_curr_msg);

        }
        else if (m_game_state == game_state_e::FINISHED_PUZZLE) {

            title_render("RESULT MATCH");
            display_check(game_actions.back());
            std::string checks;
            checks = std::to_string(m_checks_left);
            std::cout << Color::tcolor(">>> Checks left [ " + checks, Color::BRIGHT_YELLOW);
            std::cout << Color::tcolor(" ]\n",Color::BRIGHT_YELLOW);
            std::cout << Color::tcolor(">>> Digits left [ " + m_digits_left, Color::BRIGHT_YELLOW);
            std::cout << Color::tcolor(" ]\n", Color::BRIGHT_YELLOW);
            msg_render(m_curr_msg);
        }
        
    }

    void SudokuGame::initialize(int argc, char** argv ) {

        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-h") == 0) {

                usage("");
                exit(0);
            } else if (strcmp(argv[i], "-c") == 0) {
                std::string check_num;
                std::stringstream buffer;
                buffer << argv[i + 1];
                buffer >> check_num;

                if ((i + 1) < argc) { //<! check if argument has size for verification, case not error message is  printed;

                    if((isdigit(check_num[0]) != 0) and (std::stoi(check_num) >= 0 and std::stoi(check_num) < 10)){

                        short buffer_checks = std::stoi(argv[i + 1], nullptr, 10);
                        std::cout << buffer_checks <<"\n";
                        m_opt.total_checks = buffer_checks;
                        i ++;
                    } else {

                        std::cout << ">> Please enter a valid number. \n";
                        usage("");
                        exit(0);
                    }
                } else {

                    std::cout << ">> Number of checks not entered.\n";
                    usage("");
                    exit(0);
                }
            } else {
                m_opt.input_filename = argv[i]; 
            }
        }
     
        reading_boards(m_opt.input_filename);
        init_m_solution();
        init_m_playerboard();
        game_actions.push_back(m_playerboard);
        m_checks_left = m_opt.total_checks;

    }

    void SudokuGame::display_welcome (void) const {
        std::cout << Color::tcolor( "=================================================", Color::BRIGHT_BLUE ) << std::endl;
        std::cout << Color::tcolor( "  Welcome to a terminal version of Sudoku, v1.0  ", Color::BRIGHT_BLUE ) << std::endl;
        std::cout << Color::tcolor( "  Copyright (C) 2020, Selan R. dos Santos", Color::BRIGHT_BLUE ) << std::endl;
        std::cout << Color::tcolor( "=================================================", Color::BRIGHT_BLUE ) << std::endl;
        std::cout << std::endl;
        std::cout << Color::tcolor( ">>> Press enter to start.\n", Color::GREEN ) << std::endl;
        std::cin.ignore();
    }

    void SudokuGame::reading_boards(std::string file_local) {
        //##### LER ARQUIVO DE TABULEIROS
        std::ifstream file; //< An ifstream object to open and access the file

        file.open(file_local);

        std::cout << Color::tcolor(">>> Opening input file [ " + file_local + " ].", Color::BRIGHT_GREEN) << std::endl;

        if (file.fail()) { //< Exit if any file occurs
            std::cout << Color::tcolor(">>> File failed to open.", Color::BRIGHT_RED) << std::endl;
            exit(0);
        }

        std::stringstream buffer;      //< Creat a stringstream object
        buffer << file.rdbuf();        //< Reads entire contents of the file into the strigstream buffer
        std::string file_contents;     //< String to store file content

        file.close();

        std::cout << Color::tcolor(">>> Processing data, please wait.", Color::BRIGHT_GREEN) << std::endl;

        std::vector<std::string> tokens; //< Store lines of input

        while (std::getline(buffer, file_contents, '\n')) { //Tokenizer string with all file content
            tokens.push_back(file_contents);
        }

        SBoard buffer_board;

        short current_line{0};
        int tokens_size = tokens.size();
        for (int i = 0; i < tokens_size; i++) {


            if(tokens[i].empty()){
                continue;
            } else {

                std::vector<std::string> tokens_line;
                std::string line_contents;
                std::stringstream buffer_line;
                buffer_line << tokens[i];

                while (std::getline(buffer_line, line_contents, ' ')) { //Tokenizer line

                    tokens_line.push_back(line_contents);
                }

                //std::cout << ">> NUMBER OF ELEMENTS IN LINE: " << tokens_line.size() << "\n";

                for (short j = 0; j < SB_SIZE; j++) { //!< Loop in col of elements in the current line

                    std::stringstream element_buffer;
                    short current_element;

                    element_buffer << tokens_line[j];  //<! Conversion to short
                    element_buffer >> current_element;

                    buffer_board.board[current_line][j] = current_element;

                    //std::cout << buffer_board.board[current_line][j] << " ";
                }
                //std::cout << "\n";

                current_line++;

                if (current_line == 9) {

                    current_line = 0;
                    if (is_valid(buffer_board)) {
                        boards_list.push_back(buffer_board);
                    }
                }
            }
        }
        std::cout << Color::tcolor(">>> Finished reading input data", Color::BRIGHT_GREEN) << std::endl;
    }

    bool SudokuGame::is_valid( SBoard b ) {
        bool numbers[SB_SIZE];
        std::fill(numbers, numbers+SB_SIZE, false);

        // Testando cada linha.
        for(short row{0}; row<SB_SIZE; ++row){
            // "Cadastrar" cada elemento como presente na linha
            for (short col{0}; col<SB_SIZE; ++col){
                auto n=std::abs(b.board[row][col]);
                // Check if we've got a number outside the valid range.
                if(n<1 or n>9) return false;
                numbers[n-1] = true;
            }
            // Testar se todos os algorismos foram encontrados
            if( not std::all_of(numbers, numbers+SB_SIZE, 
                    [](bool e){return e==true;} ) ){
                return false;
            }
            // "Resetar" o hash de booleanos.
            std::fill(numbers, numbers+SB_SIZE, false);

        }

        // Testando cada coluna

        for(short col{0}; col<SB_SIZE; ++col){
            // "Cadastrar" cada elemento como presente na linha
            for (short row{0}; row<SB_SIZE; ++row){
                auto n=std::abs(b.board[row][col]);
                // Check if we've got a number outside the valid range.
                if(n<1 or n>9) return false;
                numbers[n-1] = true;
            }
            // Testar se todos os algorismos foram encontrados
            if( not std::all_of(numbers, numbers+SB_SIZE, 
                    [](bool e){return e==true;} ) ){
                return false;
            }
            // "Ressetar" o hash de booleanos.
            std::fill(numbers, numbers+SB_SIZE, false);
        }

        //!< Testando quadrante

        for (int initial_row = 0; initial_row < SB_SIZE; initial_row+=3) {

            for (int initial_col = 0; initial_col < SB_SIZE; initial_col+=3) {

                for (int row = initial_row; row < initial_row + 3; row++) {

                    for (int col = initial_col; col < initial_col + 3; col++) {
                        auto n = std::abs(b.board[row][col]);
                        // Check if we've got a number outside the valid range.
                        if(n<1 or n>9) return false;
                        numbers[n-1] = true;
                        //std::cout << " m[" << row << "] [" << col << "] = " << b[row][col] << "\n"; 
                    }
                }

                if( not std::all_of(numbers, numbers+SB_SIZE,
                [](bool e){return e==true;} ) ){
                    return false;
                }   
            }  
        }

        return true; // This is just a stub. Replace it as needed.
    }

    bool SudokuGame::game_over() {
        if(game_end)
            return true;
        else 
            return false;
    }

    void SudokuGame::board_render(SBoard b) const { 

        std::cout << "     1 2 3   4 5 6   7 8 9\n";

        for (short row{0}; row < SB_SIZE; row++) {

            if(row%3 == 0) std::cout << Color::tcolor("   +-------+-------+-------+\n", Color::BRIGHT_BLUE );

            for (short col = 0; col < SB_SIZE; col++){

                if (col == 0) std::cout << " " << row + 1 << " ";
                if (col%3 == 0) std::cout << Color::tcolor("| ",Color::BRIGHT_BLUE );
                if (b.board[row][col] == 0) {
                    std::cout << " " << " ";
                } 
                else if(b.board[row][col] > 0 and b.board[row][col] <= 9) {

                    std::string print = std::to_string(std::abs(b.board[row][col]));
                    std::cout << Color::tcolor( print, Color::WHITE) << " ";
                } 
                else if (b.board[row][col] > 10 and b.board[row][col] <= 19) {

                    std::string print = std::to_string(std::abs(b.board[row][col] - 10));
                    std::cout << Color::tcolor( print, Color::CYAN) << " ";
                } 
                else if (b.board[row][col] > 20 and b.board[row][col] <= 29) {

                    std::string print = std::to_string(std::abs(b.board[row][col] - 20));
                    std::cout << Color::tcolor( print, Color::CYAN) << " ";
                }
            }
            std::cout << Color::tcolor("| \n",Color::BRIGHT_BLUE );;

        }
        std::cout << Color::tcolor("   +-------+-------+-------+\n",Color::BRIGHT_BLUE );

    }

    void SudokuGame::display_check(SBoard b) const {

        std::cout << "     1 2 3   4 5 6   7 8 9\n";

        for (short row{0}; row < SB_SIZE; row++) {

            if(row%3 == 0) std::cout << Color::tcolor("   +-------+-------+-------+\n", Color::BRIGHT_BLUE );

            for (short col = 0; col < SB_SIZE; col++){

                if (col == 0) std::cout << " " << row + 1 << " ";
                if (col%3 == 0) std::cout << Color::tcolor("| ",Color::BRIGHT_BLUE );
                if (b.board[row][col] == 0) {
                    std::cout << " " << " ";
                } 
                else if(b.board[row][col] > 0 and b.board[row][col] <= 9) {

                    std::string print = std::to_string(std::abs(b.board[row][col]));
                    std::cout << Color::tcolor( print, Color::BRIGHT_BLACK) << " ";
                } 
                else if (b.board[row][col] > 10 and b.board[row][col] <= 19) {

                    std::string print = std::to_string(std::abs(b.board[row][col] - 10));
                    std::cout << Color::tcolor( print, Color::BRIGHT_GREEN) << " ";
                } 
                else if (b.board[row][col] > 20 and b.board[row][col] <= 29) {

                    std::string print = std::to_string(std::abs(b.board[row][col] - 20));
                    std::cout << Color::tcolor( print, Color::RED) << " ";
                }
                
            }
            std::cout << Color::tcolor("| \n",Color::BRIGHT_BLUE );;

        }
        std::cout << Color::tcolor("   +-------+-------+-------+\n",Color::BRIGHT_BLUE );
    }

    void SudokuGame::title_render(std::string title_state) const {
        std::cout << Color::tcolor("\n\n\n|--------[ " + title_state + " ]--------|", Color::BRIGHT_BLUE) << std::endl << std::endl;
    }

    void SudokuGame::display_play_cmd(void) const {

        std::cout << Color::tcolor("Commands syntax:\n", Color::BRIGHT_GREEN);
        std::cout << Color::tcolor("	'enter' (without typing anything)   -> go back to previous menu.\n", Color::BRIGHT_GREEN);
        std::cout << Color::tcolor("	'p' <row> <col> <number> + 'render' -> place <number> on board at location (<row>, <col>).\n", Color::BRIGHT_GREEN);
        std::cout << Color::tcolor("	'r' <row> <col> + 'enter'           -> remove number on board at location (<row>, <col>).\n", Color::BRIGHT_GREEN);
        std::cout << Color::tcolor("	'c' + 'enter'                       -> check which moves made are correct.\n", Color::BRIGHT_GREEN);
        std::cout << Color::tcolor("	'u' + 'enter'                       -> undo last play.\n", Color::BRIGHT_GREEN);
        std::cout << Color::tcolor("	<row>, <col>, <number> must be in range [ 1,9].\n\n", Color::BRIGHT_GREEN);
        std::cout << Color::tcolor("    Enter command > " + m_checks_left, Color::BRIGHT_YELLOW);
    }

    void SudokuGame::msg_render(std::string msg) const {
        std::cout << Color::tcolor(">>> MSG [ " + msg + " ]", Color::BRIGHT_YELLOW) << "\n\n";;
    }

    void SudokuGame::menu_render(void) const {
        std::cout << Color::tcolor(" 1-PLAY 2-NEW GAME 3-QUIT 4-HELP ", Color::BRIGHT_MAGENTA) << std::endl;
        std::cout << Color::tcolor(" Select option [1,4] >  ", Color::BRIGHT_MAGENTA);
    }

    void SudokuGame::display_help(void) const {
        std::cout << Color::tcolor("---------------------------------------------------------------------------\n", Color::BRIGHT_GREEN);
        std::cout << Color::tcolor("   The goal of Sudoku is to fill a 9x9 grid with numbers so that each now,\n", Color::BRIGHT_GREEN);
        std::cout << Color::tcolor("   column and 3x3 section (nonet) contain all of the digits between 1 and 9.\n\n", Color::BRIGHT_GREEN);
        std::cout << Color::tcolor("   The Sudoku rules are:\n", Color::BRIGHT_GREEN);
        std::cout << Color::tcolor("   1. Each row, column, and nonet can contain each number (typically 1 to 9)\n", Color::BRIGHT_GREEN);
        std::cout << Color::tcolor("      exactly once.\n", Color::BRIGHT_GREEN);
        std::cout << Color::tcolor("   2. Tem sum of all numbers in any nonet, row, or column must be equal to 45.\n", Color::BRIGHT_GREEN);
        std::cout << Color::tcolor("----------------------------------------------------------------------------\n\n", Color::BRIGHT_GREEN);
        std::cout << Color::tcolor("Press enter to go back.", Color::BRIGHT_GREEN); 
    }
    
    void SudokuGame::new_board(void) {
        game_actions.clear(); //<! Delete old boards in game list

        int size_of_list =  boards_list.size();
        std::cout <<  "\n" << boards_list.size() << "\n";

        if ((m_current_boardlist + 1) < (size_of_list - 1)/*Last board of list*/) { //!< if current board selected in list is not the last
            m_current_boardlist ++;
            init_m_solution();
            init_m_playerboard();
            game_actions.push_back(m_playerboard);

        } else { //!< if the selected board goes beyond the list, it returns to the first
            m_current_boardlist = 0;
            init_m_solution();
            init_m_playerboard();
            game_actions.push_back(m_playerboard);

        }
        
        
    }

    void SudokuGame::init_m_solution(void) {

        m_solution = boards_list[m_current_boardlist];
    }

    void SudokuGame::init_m_playerboard(void) {
        for (short i = 0; i < SB_SIZE; i++) {
            for (short j = 0; j < SB_SIZE; j++) {
                if(m_solution.board[i][j] > 0  and m_solution.board[i][j] <= SB_SIZE) {
                    m_playerboard.board[i][j] = m_solution.board[i][j];
                } else {
                    m_playerboard.board[i][j] = 0;
                }
            }
            
        }
        
    }

    void SudokuGame::game_play(void) {
        
        m_playerboard = game_actions.back();

        std::string input_gameplay;
        std::getline(std::cin, input_gameplay);

        if (input_gameplay[0] == 'p') {
            if (input_gameplay.size() == 7) { //!< If input is correct: 3(spaces) + 1(prefix action) + 2(position row col) + 1(number input)
                game_place(input_gameplay);
            } else {
                m_curr_msg = {"Invalid input."};
            }
            
        } else if (input_gameplay[0] == 'r') {
            if (input_gameplay.size() == 5) { //!< If input is correct: 2(spaces) + 1(prefix action) + 2(position row col)
                game_remove(input_gameplay);
            } else {
                m_curr_msg = {"Invalid input."};
            }
        } else if (input_gameplay[0] == 'c') {
            if (m_checks_left > 0) {   
                checking = true;
                m_checks_left --;
                m_curr_msg = {"Cheking done. Press enter to continue."};

            } else {
                m_curr_msg = {"Your checks are over. ಠ‿ಠ "};
            }
            

        } else if (input_gameplay[0] == 'u') {
            game_undo();
        } else {
                m_curr_msg = {"Invalid input."};
        }
        
    }

    void SudokuGame::game_place(std::string line){
        
        std::string row, col, num;

        row = line[2];
        col = line[4];
        num = line[6];

        short srow = std::stoi(row, nullptr, 10);
        short scol = std::stoi(col, nullptr, 10);
        short snum = std::stoi(num, nullptr, 10);


        if (m_playerboard.board[srow-1][scol-1] == 0) {

            if (std::abs(m_solution.board[srow-1][scol-1])
             == snum) {

                m_playerboard.board[srow-1][scol-1]
                 = snum + 10;

                game_actions.push_back(m_playerboard);
            } 
            else if (std::abs(m_solution.board[srow-1][scol-1])
             != snum) {

                m_playerboard.board[srow-1][scol-1]
                 = snum + 20;
                
                game_actions.push_back(m_playerboard);
            }
        } else {
            m_curr_msg = {"Invalid move."};
        }
    }

    void SudokuGame::game_remove(std::string line) {
        std::string row, col;

        row = line[2];
        col = line[4];

        short srow = std::stoi(row, nullptr, 10);
        short scol = std::stoi(col, nullptr, 10);


        if (m_playerboard.board[srow-1][scol-1] > 10 and m_playerboard.board[srow-1][scol-1] < 30 ) {


            m_playerboard.board[srow-1][scol-1] = 0;
            game_actions.push_back(m_playerboard);

        } else {
            m_curr_msg = {"Invalid move."};
        }
    }

    void SudokuGame::game_undo(void) {
        game_actions.pop_back();
    }

    void SudokuGame::fucking_title(void) {
        std::cout << Color::tcolor( "  ██████  █    ██ ▓█████▄  ▒█████   ██ ▄█▀ █    ██ ", Color::BRIGHT_RED ) << std::endl;
        std::cout << Color::tcolor( "▒██    ▒  ██  ▓██▒▒██▀ ██▌▒██▒  ██▒ ██▄█▒  ██  ▓██▒", Color::BRIGHT_RED ) << std::endl;
        std::cout << Color::tcolor( "░ ▓██▄   ▓██  ▒██░░██   █▌▒██░  ██▒▓███▄░ ▓██  ▒██░", Color::BRIGHT_RED ) << std::endl;
        std::cout << Color::tcolor( "  ▒   ██▒▓▓█  ░██░░▓█▄   ▌▒██   ██░▓██ █▄ ▓▓█  ░██░", Color::BRIGHT_RED ) << std::endl;
        std::cout << Color::tcolor( "▒██████▒▒▒▒█████▓ ░▒████▓ ░ ████▓▒░▒██▒ █▄▒▒█████▓ ", Color::BRIGHT_RED ) << std::endl;
        std::cout << Color::tcolor( "▒ ▒▓▒ ▒ ░░▒▓▒ ▒ ▒  ▒▒▓  ▒ ░ ▒░▒░▒░ ▒ ▒▒ ▓▒░▒▓▒ ▒ ▒ ", Color::BRIGHT_RED ) << std::endl;
        std::cout << Color::tcolor( "░ ░▒  ░ ░░░▒░ ░ ░  ░ ▒  ▒   ░ ▒ ▒░ ░ ░▒ ▒░░░▒░ ░ ░ ", Color::BRIGHT_RED ) << std::endl;
        std::cout << Color::tcolor( "░  ░  ░   ░░░ ░ ░  ░ ░  ░ ░ ░ ░ ▒  ░ ░░ ░  ░░░ ░ ░ ", Color::BRIGHT_RED ) << std::endl;
        std::cout << Color::tcolor( "      ░     ░        ░        ░ ░  ░  ░      ░     ", Color::BRIGHT_RED ) << std::endl;
        std::cout << Color::tcolor( "                   ░                               ", Color::BRIGHT_RED ) << std::endl;
    }

    bool SudokuGame::board_isfull() {
        bool board_full{true};

        for (short i = 0; i < SB_SIZE; i++) {
            for (short j = 0; j < SB_SIZE; j++) {
                if (m_playerboard.board[i][j] == 0) {
                    board_full = false;
                }
            }
        }

        if (board_full == true) {
            return true;
        }
        else {
            return false;
        }    
    }
 
    void SudokuGame::game_result(void) {
            bool win{true};

            for (short i = 0; i < SB_SIZE; i++) {
                for (short j = 0; j < SB_SIZE; j++) {
                    
                    if (std::abs(m_solution.board[i][j]) != m_playerboard.board[i][j]) {

                        win = false;
                    }
                }
            }
            
            if (win) {
                m_curr_msg = {"You win!"};
            } else {
                m_curr_msg = {"You are a loser! Hahahahah"};
            }
    }

}
