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
yes | cp -f "$folder_path/build.gradle" "./build.gradle"
echo "Replaced build.gradle file."

# Replace config.h file
yes | cp -f "$folder_path/config.h" "./src/giac/headers/config.h"
echo "Replaced config.h file."

# Replace simpleInterface folder
cp -r -f "$folder_path/simpleInterface" "./src/"
echo "Replaced simpleInterface folder."

# Coppy TomMath .c and .h files
cp -r -f "$folder_path/tommath" "./src/"
yes | cp -r -f "./giacbuilt_tommath_additions/tommath/headers/" "./src/giac/headers/"
echo "Copied TomMath sources and headers."

# Remove ./build folder
if [ -d "./build" ]; then
   rm -rf "./build"
fi

# Call the Gradle task to create iOS XCFramework
./gradlew createIosXcframework

# Pack the framework
pushd build/libs/framework/
zip -r -y Giac.xcframework.zip ./Giac.xcframework/
popd

# reset the local folder to make sure -> don't upload any file to public repo.
git reset --hard

echo "Finished creating iOS XCFramework. Please copy the generated XCFramework in ./build/libs/framework/Giac.xcframework and ./build/libs/framework/Giac.xcframework.zip to your iOS project."
