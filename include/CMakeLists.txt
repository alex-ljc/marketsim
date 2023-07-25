
add_library(alex_library STATIC LimitOrderBook.cpp LimitOrderBook.h Data.h Exchange.cpp Exchange.h SenderReceivers.cpp SenderReceivers.h TradingBot.cpp TradingBot.h Information.cpp Information.h)



target_include_directories(alex_library INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})