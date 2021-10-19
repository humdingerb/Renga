cmake_minimum_required (VERSION 2.8)

# projectname is the same as the main-executable
project(Renga)

add_definitions('-g')
add_definitions('-Wall' '-Wextra' '-Werror' '-Wno-multichar')
add_compile_options(-fdiagnostics-color=always)

add_executable(Renga
	# TODO review all these files for usefulness and relevance
	 jabber/Agent.cpp
	 jabber/UserID.cpp
	 jabber/ChatTextView.cpp
	 jabber/CustomStatusWindow.cpp
	 jabber/LoginPreferencesView.cpp
	 jabber/PreferencesWindow.cpp
	 jabber/RosterItem.cpp
	 jabber/RosterView.cpp
	 jabber/SendTalkWindow.cpp
	 jabber/SoundPreferencesView.cpp
	 jabber/TalkListItem.cpp
	 jabber/TalkManager.cpp
	 jabber/TransportItem.cpp
	 jabber/TransportPreferencesView.cpp
	 jabber/BlabberApp.cpp
	 jabber/AgentList.cpp
	 jabber/BlabberSettings.cpp
	 jabber/BuddyWindow.cpp
	 jabber/JabberSpeak.cpp
	 jabber/JRoster.cpp
	 jabber/SoundSystem.cpp
	 jabber/GenericFunctions.cpp
	 jabber/FileItem.cpp
	 jabber/StatusView.cpp
	 jabber/MessageRepeater.cpp
	 jabber/FileXMLReader.cpp
	 jabber/XMLEntity.cpp
	 jabber/XMLReader.cpp

	# Gloox extras (to be upstreamed, I guess)
	network/bob.cpp
	network/bookmark2storage.cpp
	network/media.cpp

	# "Modern" XMPP classes using gloox cleanly and directly
	network/BobStore.cpp
	network/BookmarkManager.cpp
	network/GlooxHandler.cpp

	# Misc. support stuff
	support/AppLocation.cpp
	support/main.cpp

	# "Modern" UI classes using gloox types and StartWatching/SendNotices
	ui/AboutWindow.cpp
	ui/BookmarkItem.cpp
	ui/BuddyInfoWindow.cpp
	ui/ChangeNameWindow.cpp
	ui/EditingFilter.cpp
	ui/HVIFUtil.cpp
	ui/MainWindow.cpp
	ui/ModalAlertFactory.cpp
	ui/PeopleListItem.cpp
	ui/PictureView.cpp
	ui/RegisterAccountWindow.cpp
	ui/RosterSuperitem.cpp
	ui/RotateChatFilter.cpp
	ui/TalkView.cpp
)

set_property(TARGET Renga PROPERTY CXX_STANDARD 14)

find_library(gloox gloox)
if(NOT gloox)
	message(FATAL_ERROR "gloox not found, try pkgman install devel:libgloox_x86")
endif()

find_library(expat expat)
if(NOT expat)
	message(FATAL_ERROR "expat not found, try pkgman install devel:libexpat_x86")
endif()

target_link_libraries(Renga
	be expat game gloox shared tracker translation
)

add_custom_command(TARGET Renga POST_BUILD
	COMMAND rc -o Resource.rsrc ${CMAKE_SOURCE_DIR}/jabber/Resource.rdef
	COMMAND xres -o ${CMAKE_BINARY_DIR}/Renga Resource.rsrc
		-a PiNG:2:bulb-normal  ${CMAKE_SOURCE_DIR}/resources/graphics/bulb-normal.png
		-a PiNG:3:bulb-big     ${CMAKE_SOURCE_DIR}/resources/graphics/bulb-big.png
		-a VICN:4:away-online  ${CMAKE_SOURCE_DIR}/resources/icons/away-online.hvif
		-a VICN:5:online       ${CMAKE_SOURCE_DIR}/resources/icons/online.hvif
		-a VICN:6:offline      ${CMAKE_SOURCE_DIR}/resources/icons/offline.hvif
		-a VICN:7:unknown      ${CMAKE_SOURCE_DIR}/resources/icons/unknown.hvif
		-a VICN:10:preferred   ${CMAKE_SOURCE_DIR}/resources/icons/preferred.hvif
		-a VICN:11:home        ${CMAKE_SOURCE_DIR}/resources/icons/home.hvif
		-a VICN:12:work        ${CMAKE_SOURCE_DIR}/resources/icons/work.hvif
	COMMAND mimeset -f ${CMAKE_BINARY_DIR}/Renga
	DEPENDS jabber/Resource.rdef
)

install(TARGETS Renga RUNTIME DESTINATION apps/)

include_directories(${CMAKE_SOURCE_DIR})

# Testing support

#add_executable(Test
#	tests/Test.cpp
#)

#target_link_libraries(Test be)

# YouCompleteMe support

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json")
	EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
		${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
	)
endif()
