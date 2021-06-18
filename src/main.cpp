#include <iostream>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <getopt.h>
#include "ParaTokenizer.hh"
#include "ParaOperator.hh"
#include "ParaExpression.hh"
#include "ParaSymbolTable.hh"
#include "ParaMathLibrary.hh"
#include "nask_utility.hpp"
#include "bracket_utility.hpp"
#include "string_util.hpp"
#include <ctime>

using namespace std::placeholders;

int process_each_assembly_line(char** argv,
			       std::ifstream& nas_file,
			       std::vector<uint8_t>& binout_container,
			       size_t start_line = 0) {

     long line_number = 1;
     std::string input;

     // spdlog
     auto logger = spdlog::basic_logger_mt("opennask", "debug.log");

     static nask_utility::Instructions inst;
     static std::string current_symbol = "";
     using InstAlias = nask_utility::Instructions;

     static meta::funcs_type funcs {
	  std::make_pair("\["	  , std::bind(&InstAlias::process_token_BRACKET , inst, _1, _2)),
	  std::make_pair("ADD"	  , std::bind(&InstAlias::process_token_ADD	, inst, _1, _2)),
	  std::make_pair("ALIGNB" , std::bind(&InstAlias::process_token_ALIGNB  , inst, _1, _2)),
	  std::make_pair("AND"	  , std::bind(&InstAlias::process_token_AND	, inst, _1, _2)),
	  std::make_pair("CALL"	  , std::bind(&InstAlias::process_token_CALL	, inst, _1, _2)),
	  std::make_pair("CLI"	  , std::bind(&InstAlias::process_token_CLI	, inst, _1, _2)),
	  std::make_pair("CMP"	  , std::bind(&InstAlias::process_token_CMP	, inst, _1, _2)),
	  std::make_pair("DB"	  , std::bind(&InstAlias::process_token_DB	, inst, _1, _2)),
	  std::make_pair("DD"	  , std::bind(&InstAlias::process_token_DD	, inst, _1, _2)),
	  std::make_pair("DW"	  , std::bind(&InstAlias::process_token_DW	, inst, _1, _2)),
	  std::make_pair("EXTERN" , std::bind(&InstAlias::process_token_EXTERN  , inst, _1, _2)),
	  std::make_pair("GLOBAL" , std::bind(&InstAlias::process_token_GLOBAL  , inst, _1, _2)),
	  std::make_pair("HLT"	  , std::bind(&InstAlias::process_token_HLT	, inst, _1, _2)),
	  std::make_pair("IMUL"	  , std::bind(&InstAlias::process_token_IMUL	, inst, _1, _2)),
	  std::make_pair("IN"	  , std::bind(&InstAlias::process_token_IN	, inst, _1, _2)),
	  std::make_pair("INT"	  , std::bind(&InstAlias::process_token_INT	, inst, _1, _2)),
	  std::make_pair("IRET"	  , std::bind(&InstAlias::process_token_IRET	, inst, _1, _2)),
	  std::make_pair("IRETD"  , std::bind(&InstAlias::process_token_IRET	, inst, _1, _2)),
	  std::make_pair("JAE"	  , std::bind(&InstAlias::process_token_JAE	, inst, _1, _2)),
	  std::make_pair("JBE"	  , std::bind(&InstAlias::process_token_JBE	, inst, _1, _2)),
	  std::make_pair("JB"	  , std::bind(&InstAlias::process_token_JB	, inst, _1, _2)),
	  std::make_pair("JC"	  , std::bind(&InstAlias::process_token_JC	, inst, _1, _2)),
	  std::make_pair("JE"	  , std::bind(&InstAlias::process_token_JE	, inst, _1, _2)),
	  std::make_pair("JMP"	  , std::bind(&InstAlias::process_token_JMP	, inst, _1, _2)),
	  std::make_pair("JNC"	  , std::bind(&InstAlias::process_token_JNC	, inst, _1, _2)),
	  std::make_pair("JNE"	  , std::bind(&InstAlias::process_token_JNE	, inst, _1, _2)),
	  std::make_pair("JNZ"	  , std::bind(&InstAlias::process_token_JNZ	, inst, _1, _2)),
	  std::make_pair("JZ"	  , std::bind(&InstAlias::process_token_JZ	, inst, _1, _2)),
	  std::make_pair("LGDT"	  , std::bind(&InstAlias::process_token_LGDT	, inst, _1, _2)),
	  std::make_pair("LIDT"	  , std::bind(&InstAlias::process_token_LIDT	, inst, _1, _2)),
	  std::make_pair("LTR"	  , std::bind(&InstAlias::process_token_LTR	, inst, _1, _2)),
	  std::make_pair("MOV"	  , std::bind(&InstAlias::process_token_MOV	, inst, _1, _2)),
	  std::make_pair("NOP"	  , std::bind(&InstAlias::process_token_NOP	, inst, _1, _2)),
	  std::make_pair("OR"	  , std::bind(&InstAlias::process_token_OR	, inst, _1, _2)),
	  std::make_pair("ORG"	  , std::bind(&InstAlias::process_token_ORG	, inst, _1, _2)),
	  std::make_pair("OUT"	  , std::bind(&InstAlias::process_token_OUT	, inst, _1, _2)),
	  std::make_pair("POP"	  , std::bind(&InstAlias::process_token_POP	, inst, _1, _2)),
	  std::make_pair("POPA"	  , std::bind(&InstAlias::process_token_POPA	, inst, _1, _2)),
	  std::make_pair("POPAD"  , std::bind(&InstAlias::process_token_POPA	, inst, _1, _2)),
	  std::make_pair("POPF"	  , std::bind(&InstAlias::process_token_POPF 	, inst, _1, _2)),
	  std::make_pair("POPFD"  , std::bind(&InstAlias::process_token_POPF 	, inst, _1, _2)),
	  std::make_pair("PUSH"   , std::bind(&InstAlias::process_token_PUSH 	, inst, _1, _2)),
	  std::make_pair("PUSHA"  , std::bind(&InstAlias::process_token_PUSHA 	, inst, _1, _2)),
	  std::make_pair("PUSHAD" , std::bind(&InstAlias::process_token_PUSHA 	, inst, _1, _2)),
	  std::make_pair("PUSHF"  , std::bind(&InstAlias::process_token_PUSHF 	, inst, _1, _2)),
	  std::make_pair("PUSHFD" , std::bind(&InstAlias::process_token_PUSHF 	, inst, _1, _2)),
	  std::make_pair("RET"	  , std::bind(&InstAlias::process_token_RET	, inst, _1, _2)),
	  std::make_pair("RETF"	  , std::bind(&InstAlias::process_token_RETF    , inst, _1, _2)),
	  std::make_pair("RETN"	  , std::bind(&InstAlias::process_token_RET     , inst, _1, _2)),
	  std::make_pair("RESB"	  , std::bind(&InstAlias::process_token_RESB	, inst, _1, _2)),
	  std::make_pair("SHR"	  , std::bind(&InstAlias::process_token_SHR	, inst, _1, _2)),
	  std::make_pair("STI"	  , std::bind(&InstAlias::process_token_STI	, inst, _1, _2)),
	  std::make_pair("SUB"	  , std::bind(&InstAlias::process_token_SUB	, inst, _1, _2)),
	  std::make_pair("XOR"	  , std::bind(&InstAlias::process_token_XOR	, inst, _1, _2))
       };

     if (start_line != 0) {
	  // 시작 위치로 변경
	  line_number = start_line;
	  for (size_t i = 0; i < start_line - 1; i++) {
	       std::getline(nas_file, input);
	  }
     }

     for (; std::getline(nas_file, input); line_number++) {

	  // 라인 번호 체크
	  logger->info("{}: {}", line_number, input);

	  // 심볼 테이블에 심볼 정보를 유지
	  if (nask_utility::starts_with(input, "_")) {
	       using namespace std::regex_constants;
	       const std::regex symbol_reg("^(_[a-z0-9_]*):[^:]*$", extended | icase);
	       std::smatch match;

	       if (std::regex_search(input, match, symbol_reg) && match.size() > 1) {
		    if (current_symbol == "") {
			 inst.symbol_offsets["_"] = 0; // placeholder
			 inst.symbol_offsets[match[1].str()] = 0;
			 current_symbol = match[1].str();
			 logger->info("[detect] symbol: {}, offs size: {}", current_symbol, 0);
		    } else {
			 const size_t already_counted = inst.symbol_offsets["_"];
			 logger->info("[detect] symbol: {}, offs size: {}", current_symbol, already_counted);
			 inst.symbol_offsets[match[1].str()] = already_counted;
			 current_symbol = match[1].str();
		    }
	       }
	  }

	  // 실 코드가 아닌 라벨일 경우 (CRLF끝일때는 예외적이라 어떻게 해야 할까)
	  if (nask_utility::ends_with(input, ":") || nask_utility::ends_with(input, ":\r")) {
	       std::string label_dst = input.substr(0, input.find(":", 0));
	       logger->info("coming another label: {} bin[{}]",
			    label_dst, std::to_string(binout_container.size()));

               // label: (label_dst라고 함)
               // 1) label_dst의 위치를 기록함 -> label_dst_stack
               // 2) 같은 이름의 label_src가 저장되어 있으면, 오프셋값을 계산하고 종료
	       inst.store_label_dst(label_dst, binout_container);
	       inst.update_label_dst_offset(label_dst, binout_container);
	       continue;
	  }

	  // 입력 줄에 매크로가 포함되어 있으면 다시 작성
	  for (const std::string pre_process_word : PRE_PROCESS_WORDS) {
	       std::size_t found = input.find(pre_process_word);
	       if (found != std::string::npos && pre_process_word == "EQU") {
		    logger->info("coming label EQU");
		    std::istringstream input_stream(input.c_str());
		    TParaTokenizer tokenizer(input_stream, &inst.token_table);
		    inst.process_token_EQU(tokenizer, binout_container);
		    continue;
	       }
	  }

          // 입력 줄에 사칙 계산이 포함 된 경우, 이를 다시 작성
	  std::string tmp = input;

	  if (nask_utility::is_contains_math_op(input)) {
	       // EQU에서 정의 된 문자열이 있으면 변환 해 둔다
	       tmp = inst.try_replace_equ(input);
	       input = nask_utility::expr_math_op(tmp);
	       logger->info("{}: {} // *** math op replaced ***", line_number, input);
	  } else {
	       //
	       // 위에서 처리하지 못한 것을 뭐라도 한다
	       //
	       tmp = inst.try_replace_equ(input);
	       if (nask_utility::is_contains_math_op(tmp)) {
		    logger->info("{}: {} // *** suspicious line ***", line_number, input);
		    tmp = nask_utility::replace_hex2dec(tmp);
		    logger->info("{}: {} // *** replace hex to dec ***", line_number, tmp);
		    input = nask_utility::expr_math_op(tmp);
		    logger->info("{}: {} // *** EQU & math op are replaced ***", line_number, input);
	       }
	  }

	  // 입력 행을 istream에 넣고 토크나이저를 생성
	  std::istringstream input_stream(input.c_str());
	  TParaTokenizer tokenizer(input_stream, &inst.token_table);
	  TParaToken token;

	  if (nask_utility::starts_with(input, "\[")) {
	       logger->info("eval bracket");
	       try {
		    int r = inst.process_token_BRACKET(tokenizer, binout_container);
		    if (r != 0) {
			 // 오류가있는 행을 표시
			 logger->info(input);
			 return r;
		    }
	       } catch (TScriptException te) {
		    std::cerr << te << std::endl;
	       }
	       logger->info("eval bracket");
	       continue;
	  }

	  while (! (token = tokenizer.Next()).IsEmpty()) {
	       if (nask_utility::is_comment_line(inst.token_table, token) ||
		   nask_utility::is_line_terminated(inst.token_table, token)) {
		    break;
	       } else if (token.Is(",")) {
		    continue;
	       } else {
		    auto itr = std::find_if(std::begin(instruction),
					    std::end(instruction),
					    [&token] (const INST_TABLE& inst) -> bool {
						 return nask_utility::equals_ignore_case(
						      inst.opecode, token.AsString()
						      );
					    });

		    if( itr != instruction.end() ) {
			 //
			 // opcode를 처리하는 함수를 구해 그대로 실행
			 // (소문자도 제대로 처리를 할 수 있게)
			 //
			 meta::funcs_type::iterator it = funcs.find(token.AsString());
			 if (it != funcs.end()) {
			      logger->info("eval {}", token.AsString());

			      try {
				   const size_t before_process_size = binout_container.size();
				   const int r = it->second(tokenizer, binout_container);
				   const size_t offs = binout_container.size() - before_process_size;
				   if (current_symbol != "") {
					const size_t s = inst.symbol_offsets["_"];
					inst.symbol_offsets["_"] = s + offs;
					logger->info("symbol: {}, offs size: {}", current_symbol, s + offs);
				   }

				   if (r != 0) {
					// 오류가있는 행을 표시
					logger->info(input);
					return r;
				   }
			      } catch (TScriptException te) {
				   std::cerr << te << std::endl;
			      }
			      logger->info("eval {} end", token.AsString());
			 } else {
			      // 옵코드를 찾았지만 모르는 것일 경우
			      std::cerr << "opennask ignored unknown instruction " << token.AsString() << std::endl;
			 }
		    }
	       }
	  }
     }

     // section 테이블 추가
     if (inst.exists_section_table) {
	  nask_utility::process_section_table(inst, binout_container);
     }

     logger->info("final size: bin[{}]", binout_container.size());
     return 0;
}

int main(int argc, char** argv) {

     int opt, i, option_index;

     struct option long_options[] = {
	  { "version", no_argument, NULL, 'v' },
	  { "help",    no_argument, NULL, 'h' },
	  { 0, 0, 0, 0 }// 배열의 마지막은 모두 0으로 채운다
     };

     while((opt = getopt_long(argc, argv, "mes:", long_options, &option_index)) != -1){
	  switch(opt){
	  case 'v':
	       printf("opennask " OPENNASK_VERSION "\n");
	       printf("Copyright (C) 2016 Hiroyuki Nagata.\n"
		      "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.\n"
		      "This is free software: you are free to change and redistribute it.\n"
		      "There is NO WARRANTY, to the extent permitted by law.\n"
		      "\n"
		      "Thank you osask project !\n");
	       return 0;
	  case 'h':
	       printf("usage:  [--help | --version] source [object/binary] [list]\n");
	       return 0;
	       // 해석 할 수 없는 옵션이 발견 된 경우 "?"를 반환
	       // 옵션 인수가 부족한 경우에도 "?"를 반환
	  case '?':
	       printf("unknown or required argument option -%c\n", optopt);
	       printf("usage:  [--help | --version] source [object/binary] [list]\n");
	       return 1;   // exit(EXIT_FAILURE);와 같음 - http://okwave.jp/qa/q794746.html
	  }
     }

     if (argc - optind < 2 || argc - optind > 4) {
	  std::cerr << "usage:  [--help | --version] source [object/binary] [list]" << std::endl;
	  return 16;
     }
     const char* assembly_src = argv[optind];
     const char* assembly_dst = argv[optind + 1];

     const size_t len = nask_utility::filesize(assembly_src);
     if (len == -1) {
	  std::cerr << "NASK : can't open " << assembly_src << std::endl;
	  return 17;
     }

     // 입력 어셈블러 정보
     std::ifstream nas_file;
     nas_file.open(assembly_src);
     if ( nas_file.bad() || nas_file.fail() ) {
	  std::cerr << "NASK : can't read " << assembly_src << std::endl;
	  return 17;
     }

     // 출력하는 바이너리 정보
     std::vector<uint8_t> binout_container;
     std::ofstream binout(assembly_dst, std::ios::trunc | std::ios::binary);
     if ( binout.bad() || binout.fail() ) {
	  std::cerr << "NASK : can't open " << assembly_dst << std::endl;
	  return 17;
     }

     // 입력로드 및 분석, 평가의 루프
     process_each_assembly_line(argv, nas_file, binout_container);

     // output binary
     binout.write(reinterpret_cast<char*>(binout_container.data()), binout_container.size());
     binout.close();

     return 0;
}
