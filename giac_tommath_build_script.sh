#!/bin/bash

# Define the folder name and repository URL
folder_path="./giacbuilt_tommath_additions"
repository_url="git@github.com:GoodNotes/giacbuilt_tommath_additions.git"

# Check if the folder exists
if [ -d "$folder_path" ]; then
    echo "Folder '$folder_path' already exists."
else
    # Clone the repository
    git clone "$repository_url"
    echo "Cloned '$folder_name' from '$repository_url'."
fi

# Replace build.gradle file
cp "$folder_name/build.gradle" "./build.gradle"
echo "Replaced build.gradle file."

# Replace config.h file
cp "$folder_name/config.h" "./src/giac/headers/config.h"
echo "Replaced config.h file."

# Replace simpleInterface folder
cp -R "$folder_path/simpleInterface" "./src/"
echo "Replaced simpleInterface folder."

# Replace TomMath folder
cp -R "$folder_path/tommath" "./src/"
echo "Replaced tommath folder."

# Call the Gradle task to create iOS XCFramework
./gradlew createIosXcframework

echo "Finished creating iOS XCFramework. Please copy the generated XCFramework in ./build/libs/framework/Giac.xcframework to your iOS project."