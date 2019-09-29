cmake_minimum_required (VERSION 2.8)

# projectname is the same as the main-executable
project(Renga)

add_definitions('-g')
add_definitions('-Wall' '-Wextra' '-Werror' '-Wno-multichar')
#add_definitions('-std=c++11')

add_executable(Renga
	 jabber/Agent.cpp
	 jabber/CommandMessage.cpp
	 jabber/UserID.cpp
	 jabber/AboutWindow.cpp
	 jabber/BetterTextView.cpp
	 jabber/BuddyInfoWindow.cpp
	 jabber/ChangeNameWindow.cpp
	 jabber/ChatTextView.cpp
	 jabber/CustomStatusWindow.cpp
	 jabber/LoginPreferencesView.cpp
	 jabber/EditingFilter.cpp
	 jabber/MessagesPreferencesView.cpp
	 jabber/PeopleListItem.cpp
	 jabber/PreferencesWindow.cpp
	 jabber/RosterItem.cpp
	 jabber/RosterSuperitem.cpp
	 jabber/RosterView.cpp
	 jabber/RotateChatFilter.cpp
	 jabber/SendTalkWindow.cpp
	 jabber/SoundPreferencesView.cpp
	 jabber/TalkListItem.cpp
	 jabber/TalkManager.cpp
	 jabber/TalkWindow.cpp
	 jabber/TransportItem.cpp
	 jabber/TransportPreferencesView.cpp
	 jabber/BlabberApp.cpp
	 jabber/main.cpp
	 jabber/AgentList.cpp
	 jabber/BlabberMainWindow.cpp
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
	 jabber/AppLocation.cpp
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

	# "Modern" UI classes using gloox types and StartWatching/SendNotices
	ui/BookmarkItem.cpp
	ui/ModalAlertFactory.cpp
	ui/PictureView.cpp
	ui/RegisterAccountWindow.cpp
)

target_link_libraries(Renga
	be expat game gloox tracker translation
)

add_custom_command(TARGET Renga POST_BUILD 
	COMMAND rc -o Resource.rsrc ${CMAKE_SOURCE_DIR}/jabber/Resource.rdef
	COMMAND xres -o ${CMAKE_BINARY_DIR}/Renga Resource.rsrc
		-a PiNG:2:bulb-normal  ${CMAKE_SOURCE_DIR}/resources/graphics/bulb-normal.png
		-a PiNG:3:bulb-big     ${CMAKE_SOURCE_DIR}/resources/graphics/bulb-big.png
		-a PiNG:4:away-online  ${CMAKE_SOURCE_DIR}/resources/icons/away-online.png
		-a PiNG:5:online       ${CMAKE_SOURCE_DIR}/resources/icons/online.png
		-a PiNG:6:offline      ${CMAKE_SOURCE_DIR}/resources/icons/offline.png
		-a PiNG:7:unknown      ${CMAKE_SOURCE_DIR}/resources/icons/unknown.png
		-a PiNG:8:aol          ${CMAKE_SOURCE_DIR}/resources/icons/aol.png
		-a PiNG:9:icq          ${CMAKE_SOURCE_DIR}/resources/icons/icq.png
		-a PiNG:10:yahoo       ${CMAKE_SOURCE_DIR}/resources/icons/yahoo.png
		-a PiNG:11:msn         ${CMAKE_SOURCE_DIR}/resources/icons/msn.png
	COMMAND mimeset -f ${CMAKE_BINARY_DIR}/Renga
	DEPENDS jabber/Resource.rdef
)

install(TARGETS Renga RUNTIME DESTINATION apps/)

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
