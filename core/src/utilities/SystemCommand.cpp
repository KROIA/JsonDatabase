#include "utilities/SystemCommand.h"
#include <windows.h>
#include <vector>

namespace JsonDatabase
{
    namespace Internal
    {
        namespace SystemCommand
        {
            int execute(const std::string& command)
            {
                JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);
                // Convert the command to wide string
                std::wstring wideCommand(command.begin(), command.end());
                // int execResult = WinExec(("C:\\Windows\\System32\\cmd.exe /c "+command).c_str(), SW_HIDE);
                // return execResult;
                // wideCommand += L" exit";

                 // Create a STARTUPINFOW structure and set its properties
                STARTUPINFOW startupInfo;
                ZeroMemory(&startupInfo, sizeof(STARTUPINFOW));
                startupInfo.cb = sizeof(STARTUPINFOW);
                startupInfo.dwFlags = STARTF_USESHOWWINDOW;
                startupInfo.wShowWindow = SW_HIDE;  // Hide the console window

                // Create a PROCESS_INFORMATION structure
                PROCESS_INFORMATION processInfo;
                ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

                // Create the child process
                if (CreateProcessW(NULL, const_cast<LPWSTR>((L"C:\\Windows\\System32\\cmd.exe /c " + wideCommand).c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo))
                {
                    // Wait for the child process to finish
                    WaitForSingleObject(processInfo.hProcess, INFINITE);

                    // Get the exit code of the child process
                    DWORD exitCode = 0;
                    GetExitCodeProcess(processInfo.hProcess, &exitCode);

                    // Close process and thread handles
                    CloseHandle(processInfo.hProcess);
                    CloseHandle(processInfo.hThread);

                    return static_cast<int>(exitCode);
                }

                return -1; // Return -1 if the command execution fails
            }
            std::string executePiped(const std::string& command)
            {
                JD_GENERAL_PROFILING_FUNCTION(JD_COLOR_STAGE_1);

                //return "";
                //C:\\Windows\\System32\\cmd.exe
                // Create pipe for capturing the command output
                HANDLE pipeRead, pipeWrite;
                SECURITY_ATTRIBUTES pipeAttributes;
                ZeroMemory(&pipeAttributes, sizeof(SECURITY_ATTRIBUTES));
                pipeAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
                pipeAttributes.bInheritHandle = TRUE;
                pipeAttributes.lpSecurityDescriptor = NULL;

                if (!CreatePipe(&pipeRead, &pipeWrite, &pipeAttributes, 0))
                {
                    //std::cerr << "Failed to create pipe." << std::endl;
                    JD_CONSOLE_FUNCTION("Failed to create pipe\n");
                    return "";
                }

                // Set the read end of the pipe as the standard output and error
                if (!SetHandleInformation(pipeRead, HANDLE_FLAG_INHERIT, 0))
                {
                    //std::cerr << "Failed to set pipe handle information." << std::endl;
                    JD_CONSOLE_FUNCTION("Failed to set pipe handle information\n");
                    CloseHandle(pipeRead);
                    CloseHandle(pipeWrite);
                    return "";
                }

                // Create a STARTUPINFO structure and set its properties
                STARTUPINFOW startupInfo;
                ZeroMemory(&startupInfo, sizeof(STARTUPINFOW));
                startupInfo.cb = sizeof(STARTUPINFOW);
                startupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
                startupInfo.wShowWindow = SW_HIDE;  // Hide the console window
                startupInfo.hStdOutput = pipeWrite;
                startupInfo.hStdError = pipeWrite;

                // Create a PROCESS_INFORMATION structure
                PROCESS_INFORMATION processInfo;
                ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

                // Convert the command to wide string
                std::wstring wideCommand(command.begin(), command.end());

                // Create the child process
                if (CreateProcessW(NULL, const_cast<LPWSTR>((L"C:\\Windows\\System32\\cmd.exe /c " + wideCommand).c_str()), NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInfo))
                {
                    // Close the write end of the pipe since it's not needed in this process
                    CloseHandle(pipeWrite);

                    // Read the command output from the pipe
                    const int bufferSize = 4096;
                    std::vector<char> buffer(bufferSize);

                    std::string output;
                    DWORD bytesRead;

                    while (true)
                    {
                        if (!ReadFile(pipeRead, buffer.data(), bufferSize - 1, &bytesRead, NULL))
                        {
                            if (GetLastError() == ERROR_BROKEN_PIPE) // Pipe has been closed
                                break;
                            else
                            {
                                JD_CONSOLE_FUNCTION("Failed to read from the pipe\n");
                                //std::cerr << "Failed to read from the pipe." << std::endl;
                                CloseHandle(pipeRead);
                                CloseHandle(processInfo.hProcess);
                                CloseHandle(processInfo.hThread);
                                return "";
                            }
                        }

                        if (bytesRead == 0)
                            continue;

                        buffer[bytesRead] = '\0';
                        output += buffer.data();
                    }

                    // Wait for the child process to finish
                    WaitForSingleObject(processInfo.hProcess, INFINITE);

                    // Get the exit code of the child process
                    DWORD exitCode = 0;
                    GetExitCodeProcess(processInfo.hProcess, &exitCode);

                    // Close process and thread handles
                    CloseHandle(processInfo.hProcess);
                    CloseHandle(processInfo.hThread);

                    CloseHandle(pipeRead);

                    return output;
                }
                else
                {
                    //std::cerr << "Failed to execute command." << std::endl;
                    JD_CONSOLE_FUNCTION("Failed to execute command\n");
                    CloseHandle(pipeRead);
                    CloseHandle(pipeWrite);
                    return "";
                }
            }
        }
    }
}