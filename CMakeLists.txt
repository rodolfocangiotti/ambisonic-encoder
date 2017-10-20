
set(ambiEncoderSources
	source/ambiEncoderController.cpp
	source/ambiEncoderController.h
	source/ambiEncoderIDs.h
	source/ambiEncoderProcessor.cpp
	source/ambiEncoderProcessor.h
	source/factory.cpp
	source/version.h
	source/macros.h
	source/Encoder.cpp
	source/Encoder.h
	source/Ramp.cpp
	source/Ramp.h
)

set(target ambiEncoder)
smtg_add_vst3plugin(${target} ${SDK_ROOT} ${ambiEncoderSources})
target_link_libraries(${target} PRIVATE base sdk)
if(MAC)
	smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/mac/Info.plist" PREPROCESS)
	smtg_set_prefix_header(${target} "${CMAKE_CURRENT_LIST_DIR}/mac/ambiEncoderPrefix.pch" "NO")
elseif(WIN)
	target_sources(${target} PRIVATE resource/ambiEncoder.rc)
endif()
