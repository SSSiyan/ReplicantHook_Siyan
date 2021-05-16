# ReplicantHook_Siyan
<a href="https://github.com/Asiern/ReplicantHook">ReplicantHook</a> with added features and ImGui implementation through DirectX 11 Hook (kiero)
<h1>Installation</h1>
<ul>
  <li>Download "dinput8.dll" from the latest release <a href="https://github.com/SSSiyan/ReplicantHook_Siyan/releases">here</a></li>
  <li>Place "dinput8.dll" next to your game's exe, usually located at D:\Steam\steamapps\common\NieR Replicant ver.1.22474487139</li>
  <li>Open the game as you usually would through steam. After a few seconds, you'll be able to open the hook with the DELETE key</li>
  <li>You can save your settings by pressing the "Save config" button, and your settings will be automatically loaded next time you open the game</li>
</ul>
<h1>Setting up the solution</h1>
<ul>
  <li>Download and install <a href="https://www.microsoft.com/en-us/download/details.aspx?id=6812">DirectX SDK</a></li>
  <li>Open the solution on Visual Studio and open the project Properties</li>
  <li>Go to VC++ Directories -> Include Directories. Click on 'Edit' and select the Include folder <br/>located on your DirectX SDK installation path. It is generally this one: <br/>%programfiles(x86)%\Microsoft DirectX SDK (June 2010)\Include\
  <li>Now go to VC++ Directories -> Library Directories. Click on 'Edit' and select the library folder <br/> located on your DirectX SDK installation path. It is generally this one - choose x86 for 32bit and x64 for 64bit: <br/>%programfiles(x86)%\Microsoft DirectX SDK (June 2010)\Lib\</li>
  <li><b>Done!</b></li>
</ul>
<h2>Kiero</h2>
<p>You can find Kiero's official repository <a href="https://github.com/Rebzzel/kiero">here</a>
