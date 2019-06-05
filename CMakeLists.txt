cmake_minimum_required (VERSION 2.8)

# projectname is the same as the main-executable
project(Jabber)

add_definitions('-g')
add_definitions('-Wall' '-Wextra' '-Werror' '-Wno-multichar')
#add_definitions('-std=c++11')

include_directories(shared/split-pane)

add_executable(Jabber
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
	 jabber/ModalAlertFactory.cpp
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
	 jabber/BitmapButton.cpp
	 jabber/FileItem.cpp
	 jabber/StatusView.cpp
	 jabber/PictureView.cpp
	 jabber/MessageRepeater.cpp
	 jabber/KeepAlive.cpp
	 jabber/PortTalker.cpp
	 jabber/JabberSSLPlug.cpp
	 jabber/JabberSocketPlug.cpp
	 jabber/FileXMLReader.cpp
	 jabber/AppLocation.cpp
	 shared/split-pane/SplitPane.cpp
	 jabber/XMLEntity.cpp
	 jabber/XMLReader.cpp
)

target_link_libraries(Jabber
	be crypto expat game gloox network ssl tracker translation
)

add_custom_command(TARGET Jabber POST_BUILD 
	COMMAND rc -o Resource.rsrc ${CMAKE_SOURCE_DIR}/jabber/Resource.rdef
	COMMAND xres -o ${CMAKE_BINARY_DIR}/Jabber Resource.rsrc
		-a PiNG:1:jabber-title ${CMAKE_SOURCE_DIR}/resources/graphics/jabber-title.png
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
	COMMAND mimeset -f ${CMAKE_BINARY_DIR}/Jabber
	DEPENDS jabber/Resource.rdef
)

install(TARGETS Jabber RUNTIME DESTINATION apps/)
