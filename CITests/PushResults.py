import subprocess
import os
import sys

#https://212553216:c0f5cf73c806c49013c12389b4d68fb8f34d837a@github.build.ge.com/PuneTeamSharing/PV624_Test_POC'
def push_results_onto_git(folder_with_results_name, test_repository_name,
                          HTML_Repository="https://github.build.ge.com/PuneTeamSharing/PV624TestResults.git"):
    """This method pushes the results up onto the git repsotiroy"""
    print("HTML repository is {}".format(HTML_Repository))
    clone_command = "git clone https://212553216:c0f5cf73c806c49013c12389b4d68fb8f34d837a@github.build.ge.com/PuneTeamSharing/PV624TestResults.git"
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
    os.system("git push https://212553216:c0f5cf73c806c49013c12389b4d68fb8f34d837a@github.build.ge.com/PuneTeamSharing/PV624TestResults")  # Pushing it upto the main repo


if __name__ == '__main__':
    re_direct_folder = sys.argv[1]
    os.system("echo in the push restuls up section")
    # os.system("dir")
    os.chdir(re_direct_folder)
    # os.system("dir")

    f = open("Test_Reports_Name.txt", "r")
    test_resports_folder = f.readline()

    msg = "echo name of the folder is" + str(test_resports_folder)
    os.system(msg)     # Printing the name of the folder

    push_results_onto_git(test_resports_folder, "PV624TestResults")