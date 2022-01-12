# Raises issues in git hub, this relies on the user being in the correct directory
import sys
import os
import subprocess
import Check_Errors_Robot_Framework
import win32com.shell.shell as shell  # Need this to run a command as adminstrator


def get_xml_report_name():
    """ gets the name of the xml report file but first goes back a directory"""
    all_files = os.listdir(os.getcwd())

    for log_file_name in all_files:
        ans = log_file_name.find("xml")  # If ans is 0 or more that means it found it

        if ans >= 0:
            return log_file_name
            # return log_file.replace("\n", "")
    return -1



def raise_git_hub_bugs(title_list, summary_list, git_commit, online_results_location, label="Robot_Framework"):
    """Pass it the two lists to create the issue"""
    os.system("echo inside the raise git hub bugs issue")
    counter = 0     # This variable keeps track of the index and where we have gotten to

    for title in title_list:
        if check_if_issue_exist(title) == -1:
            # Issue does not exist, create it
            # issue_create_cmd = "hub issue create -m " + title
            issue_create_cmd = "hub issue create -m {} -l {}".format(title, label)
            print("Issue create command is {}".format(issue_create_cmd))
            os.system("echo creating the issues")
            ans = subprocess.check_output(issue_create_cmd, shell=True)  # Checking what issues currently exist
            os.system("echo finished creating the issue")

            try:
                detailed_msg = summary_list[counter]
                if detailed_msg != "":
                    detailed_msg += " The results location is {}. The Jenkins build failure is {}"\
                        .format(online_results_location, os.environ["BUILD_ID"])
                    issue_number = get_issue_id(title)
                    print("issue number is {} detailed msg is {}".format(issue_number, detailed_msg))
                    message_command = 'hub api repos/{owner}/{repo}/issues/' + str(issue_number) + '/comments -f body=\"' + str(detailed_msg) + '\"'
                    os.system("echo sending the message command now")
                    os.system(message_command)

            except IndexError: 
                os.system("echo The title doesn't have a matching list to go with it title is {}".format(title))
        else:
            os.system("echo inside the else for the raise git hub bugs")
            issue_number = get_issue_id(title)
            message_command = 'hub api repos/{owner}/{repo}/issues/' \
                              + str(issue_number) + '/comments -f body=\"' + "The test still fails at commit: " \
                              + str(git_commit) + "The reason for the failure this time was:" \
                              + str(summary_list[counter]) + " The location of the test is:" + str(online_results_location)\
                              + ". The Jenkins build failures is {}".format(os.environ["BUILD_ID"]) + '\"'
            os.system("echo message command is {}".format(message_command))
            os.system("echo sending the message command")
            # os.system("dir")
            os.getcwd()
            try:
                ans = os.system(message_command)
            except:
                os.system("echo The command didn't work")
            if ans != 0:
                os.system("echo Failed to send the command {}".format(message_command))
        counter += 1


def check_if_issue_exist(Check_Issue_Name):
    """This method checks to see if an issue already exists, pass it the name of the issue you want to raise 
    and it'll check if an issue with a similar name already exists
    Returns a 0 if issue exists and a -1 if it doesn't"""
    arch = subprocess.check_output("hub issue", shell=True)  # Gettting what ever file is in the current directory
    ans = arch.find(Check_Issue_Name)
    if ans >= 0:
        # Issue exists
        return 0
    else:
        return -1


def get_issue_id(name):
    """Returns the issue """
    arch = subprocess.check_output("hub issue", shell=True);  # Gettting what ever file is in the current directory
    issue_list = arch.split("\n")
    
    for issue in issue_list:
        if issue.find(name) > 0:
            # Found it
            line = issue.strip()
            line = line.replace("#", "")
            issue_number = line.split(" ")
            return issue_number[0]  # Returning the first index
    return -1


def get_last_commit():
    """This function returns the number of the last git commit that happened.
    It requires the system to be in the correct directory"""
    logs = subprocess.check_output("git log", shell=True)  # Checking all the git commits
    logs_list = logs.split("\n")    # Splitting it on the new line
    last_git_commit = logs_list[0]  # The last git commit is at the first so just getting that
    last_git_commit = last_git_commit.replace("commit", "")
    last_git_commit = last_git_commit.strip()   # getting rid of any additional white spaces at the start
    return last_git_commit


if __name__ == '__main__':
    #redirect_folder = sys.argv[1]
    # redirect_folder = "STUFF"
    # print(os.environ['PROMPT'])

    # os.chdir("..")
    redirect_folder = "CITests\PV624_Test_POC"

    #online_results_location = sys.argv[2]

    online_results_location = "https://github.build.ge.com/PuneTeamSharing/PV624TestResults/tree/master"

    os.system("echo running the section that raises the git hub bugs")
    os.chdir(redirect_folder)
    os.system("dir")
    f = open("Test_Reports_Name.txt", "r")
    test_resports_folder = f.readline()
    f.close()

    os.chdir(test_resports_folder)
    # os.system("dir")

    log_file_name = get_xml_report_name()
    print("Log file name is {}".format(log_file_name))

    if log_file_name == -1:
        # Log file was not generated
        print("Log file was not generated")
        sys.exit(100)

    os.system("echo getting the test cases that failed")
    failed_test_cases, reasons = Check_Errors_Robot_Framework.read_file_state_errors(log_file_name)

    print("Failed_Test_Case is {} and reason is {}".format(failed_test_cases, reasons))

    os.chdir("..")  # Going back one so i will be in the RTD folder
    os.chdir("..")  # Going back one so i will be in the proper folder to raise the bugs folder
    os.system("echo should be in the proper CI AND Test folder now")
    # os.system("dir")
    """
    commands = "git config --global --add hub.host github.build.ge.com"
    shell.ShellExecuteEx(lpVerb='runas', lpFile='cmd.exe', lpParameters='/c ' + commands)
    ans = os.system("hub issue")
    print("Ans is {}".format(ans))
    os.system("echo Starting To raise bugs")

    online_results_location = str(online_results_location) + "/" + str(test_resports_folder)

    print("online results location is {}".format(online_results_location))
    raise_git_hub_bugs(failed_test_cases, reasons, get_last_commit(), online_results_location)

    f = open("RF_Bugs_Raised.txt", "w")

    for current_raised_test in failed_test_cases:
        f.write(current_raised_test)
        f.write("\n")
    
    f.close()
    """

