# VRClassroom

Developed with Unreal Engine 4

## Steps

- **Follow this guide** on building Unreal Engine from source: https://www.unrealengine.com/en-US/ue4-on-github

Make sure you clone version 4.25 with this command:

```git clone --single-branch --branch 4.25 https://github.com/EpicGames/UnrealEngine.git```

- Download Visual Studio 2017 or 2019 with C++ frameworks and .NET Framework 4.6.2 targeting pack

- Run the `Setup.bat` or equivalent depending on your OS

- Then run `GenerateProjectFiles.bat` or equivalent for your OS. If you are using Visual Studio 2019 run `GenerateProjectFiles.bat -2019`

- Next, just follow the [Build Instructions](https://docs.unrealengine.com/en-US/Programming/Development/BuildingUnrealEngine/index.html)

or

- If on Linux/macOS: https://docs.unrealengine.com/en-US/Platforms/Linux/BeginnerLinuxDeveloper/SettingUpAnUnrealWorkflow/index.html#3-buildingue4onlinux

For oculus quest development, we need android development tools
[Here is a tutorial for that](https://docs.unrealengine.com/en-US/Platforms/Mobile/Android/Setup/AndroidStudio/index.html)

You may need to also use git LFS for this project incase some assets fail to load.
[Download Git LFS](https://git-lfs.github.com/) then just got to the project path in your terminal and run `git lfs pull`
<br>
This should download the additional assets that will allow the project to load.

Now that all that is done, there are two ways to test and run the program. You can have the oculus link enabled and directly hit "VR preview" to run the app.
However, for Server-Client testing, we need to run to separates programs. As such, we have to build and launch to the Oculus Quest while it is connected. Make sure the Quest is in Developer Mode for this.
After hitting "Launch", it will take some time to build especially the first time. The app should appear under "Unknown Sources" as VRClassroom.
