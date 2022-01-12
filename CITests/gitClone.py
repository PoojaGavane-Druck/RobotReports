import os
import sys
import subprocess
import datetime


def find_com_file_name():
    mydate = datetime.datetime.now()
    todays_date_padded_0 = mydate.strftime('%Y-%m-%d')
    files = [f for f in os.listdir('.') if os.path.isfile(f)]
    for f in files:
        if (f.find(todays_date_padded_0) > -1) and (f.find(".txt") > -1):
            print("the com file name is {}".format(f))
            return f


def get_last_commit():
    """This function returns the number of the last git commit that happened.
    It requires the system to be in the correct directory"""
    logs = subprocess.check_output("git log", shell=True)  # Checking all the git commits
    print("-------",logs, type(logs))
    logs_list = str(logs).split("\\n")    # Splitting it on the new line
    last_git_commit = logs_list[0]  # The last git commit is at the first so just getting that
    last_git_commit = last_git_commit.replace("b'commit ", "")
    last_git_commit = last_git_commit.strip()   # getting rid of any additional white spaces at the start
    return last_git_commit


def get_xml_report_name():
    """ gets the name of the xml report file but first goes back a directory"""
    arch = subprocess.check_output("dir", shell=True);  # Gettting what ever file is in the current directory
    arch_list = arch.split("\r\n")  # Seperating it into a list
    for log_file_name in arch_list:
        ans = log_file_name.find("xml")  # If ans is 0 or more that means it found it

        if ans >= 0:
            print("Found the log file ans is {}".format(ans))
            final_list = log_file_name.split(" ")
            log_file = final_list[-1].replace("\r", "")
            return log_file.replace("\n", "")
    return -1


def get_folder_name():
    """"This function gets the name for the folder that will store the results"""
    now = datetime.datetime.now()
    final_date = str(now.year) + "_" + str(now.month) + "_" + str(now.day) + "_" + str(now.hour) + "_" + \
                 str(now.minute) + "_" + str(now.second) + "_"
    print("Final date is {}".format(final_date))
    final_name = str(final_date) + str(get_last_commit())
    return final_name


def push_results_onto_git(folder_with_results_name, test_repository_name,
                          HTML_Repository="https://github.build.ge.com/PuneTeamSharing/PV624TestResults.git"):
    """This method pushes the results up onto the git repsotiroy"""
    print("HTML repository is {}".format(HTML_Repository))
    clone_command = "git clone https://212553216:c0f5cf73c806c49013c12389b4d68fb8f34d837a@github.build.ge.com/PuneTeamSharing/PV624_Test_POC.git"
    print("Clone command is {}".format(clone_command))
    output = subprocess.check_output(clone_command, shell=True)  # Checking all the git commits
    print("Clone command output was {}".format(output))

    # Creating a test repository folder because COPY DOESN'T WORK AS EXPECTED
    create_cmd = "mkdir " + test_repository_name + "\\" + folder_with_results_name
    print("create cmd is {}".format(create_cmd))
    os.system(create_cmd)

    # Moving the directory into the correct repository so it can push
    copy_command = "copy " + folder_with_results_name + " " + test_repository_name + "\\" + folder_with_results_name
    print("copy command is {}".format(copy_command))
    os.system(copy_command)
    os.chdir(test_repository_name)  # Going into the test respository to push the stuff
    print("cwd is {}".format(os.getcwd()))
    add_to_git_repo_cmd = "git add " + folder_with_results_name
    os.system(add_to_git_repo_cmd)

    os.system("git commit -m \"pushing results up\"")  # Committing to the local repo
    os.system("git push https://212553216:c0f5cf73c806c49013c12389b4d68fb8f34d837a@github.build.ge.com/PuneTeamSharing/PV624_Test_POC")  # Pushing it upto the main repo



def last_author():
    """
    @brief:     This method finds the name of the the last person who committed the code and writes returns there
                name
    @args:      None
    @return:    str(Name_Of_author)
    """

    output_cmd = subprocess.check_output("git.exe log")
    output_list_split = str(output_cmd).split("Author:")  # Splitting it up in terms of authors
    last_commit_person_list = str(output_list_split[1]).split("Date")
    final_author = last_commit_person_list[0]
    return final_author


if __name__ == '__main__':
    """
    if len(sys.argv) < 2:
        print("You didn't pass enough args, please state the git repository which has the Testing")
        sys.exit(100)
    """
    
    #url = sys.argv[1]
    url = "https://212553216:c0f5cf73c806c49013c12389b4d68fb8f34d837a@github.build.ge.com/PuneTeamSharing/PV624_Test_POC"
    try:
        os.chdir("CITests")
    except:
        print("Tried to go into the CI and Test directory but couldn't, probably am already there")
    os.system("dir")
    # url = "https://212727274:8cbef375ee3ca7204effa8949202b632a379e207@github.build.ge.com/TestTools/RTD_Probe_Test"
    # os.system("git push https://212553216:27f1cf66ce15a35069a5a22bcba5865d@github.build.ge.com/PuneTeamSharing/PV624_Test_POC")  # Pushing it upto the main repo

    try:
        os.system("@RD /S /Q PV624_Test_POC")  # Removing the directory before doing a pull
    except:
        print("Didn't find the RTD_Probe_Test Directory")

    # This section decides what the folder that contains the results needs to be called
    folder_name = get_folder_name()
    print("Folder name is {}".format(folder_name))

    os.system("git config --global user.name 212553216")
    os.system("git config --global user.email pooja.gavane@bhge.com")

    # Downloading the RTD project
    download_cmd = "git clone " + url
    os.system(download_cmd)

    os.chdir("PV624_Test_POC")  # Navigating into the correct directory so i can run the tests
    os.system("git pull")
    print("..............",folder_name,type(folder_name))
    make_directory_cmd = "mkdir " + str(folder_name)
    os.system(make_directory_cmd)

    print("running tests- ", str(folder_name))
    

    run_robot_framework_tests_cmd = "robot --outputdir " + folder_name + " -T Main.robot"
    result = os.system(run_robot_framework_tests_cmd)  # Running the actual tests
    print("finished running tests")

    # Moving the text file now
    os.system("echo moving the text file now")
    text_file_name = find_com_file_name()
    print(text_file_name)
    os.system("dir")    # Printing out the current contents of the directory
    move_cmd = ("echo N | copy /-Y \"{}\" {}" ).format(text_file_name, folder_name)
    os.system("echo the move cmd to move the com port is {}".format(move_cmd))
    ans = os.system(move_cmd)
    if ans != 1:
        os.system("echo Failed the move command {}".format(move_cmd))

    f = open("Test_Reports_Name.txt", "w")
    f.write(folder_name)
    f.close()

    if result != 0:
        # Failed
        os.system("echo Failed The tests inside the run robot tests python script")
        os.chdir("..")  # Going back 1 directory so i know the name of the person who committed the code onto the correct branch
        # finding the last person to comit
        last_author_name = last_author()  # Getting the name of the last author
        print("name of the person who committed last is{}".format(last_author_name))
        just_email = last_author_name.split("<")
        final_email = just_email[1]
        final_email = final_email.replace(">", "")
        final_email = final_email.replace("\r", "")
        final_email = final_email.replace("\n", "")
        print("The email is,{}".format(final_email))

        try:            # Trying to re-enter the CI_And_Test folder to create the text file
            os.chdir("CITests")
        except:
            print("Couldn't re-enter the CI_And_Test file")

        f = open("Failed_Culprit.txt", "w")
        f.write(final_email)
        f.close()
        os.system("echo Should've created the failed culprit file successfully")
        os.system("dir")

    if result != 0:
        sys.exit(100)

    os.chdir("..")  # Going back to the original directory
