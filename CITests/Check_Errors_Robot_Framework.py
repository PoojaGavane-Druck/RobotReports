# This file checks to see if any errors occurred when the ROBOT FRAMEWORK test scripts were ran
import sys
import os


def read_file_state_errors(file_name):
    """
    @info:                  1) looks for a tag "<test" if it finds it it knows it needs to check if it fails or not
                            2) Then it checks to see if the any of th lines have the word "status=\"Fail\"" and
                                "critical=\"yes\"" on it. If they do, it knows that it has found the, FAIL message.
                                I've had to add an additional step because somtimes "<\status>" is on the next line
    :param file_name:
    :return:
    """
    # Need to have a check in place to see if the file actually exists

    fp = open(file_name, "r")   # opening the file in read mode only

    test_tag = 0        # Keeps track to see if a tag has been read
    status_tag = 0      # Keeps track if the status tag has been found
    status_str = ""     # The fail message that has been recorded

    failed_test = 0     # Set this variable to 1 if a test fails
    failed_test_list = []   # This list keeps tracks of all the failed tests
    failed_status_list = []     # This stores the statuses
    test_name = "Not used"

    for read_line in fp:
        # Cycling through the entire file reading evey line
        ans = str(read_line).find("<test")  # Searching for the test tag
        if ans == 0:
            # Found the test tag
            test_name = read_line   # Making a record of the test name
            test_tag = 1

        if (str(read_line).find("status=\"FAIL\"") != -1) and (read_line.find("critical=\"yes\"") != -1):
            # Checking to see if a failure has occured and if so telling the system to record the failures
            # status_tag = 1
            if read_line != "":
                status_str += read_line
                status_tag = 1

        if status_tag == 1:
            if read_line.find("</status>") != -1:
                if status_str != read_line:         # Have this step to make sure that i'm not duplicating the line
                    status_str += read_line
                # print("hel")
        #         status_str += read_line

        if test_tag == 1:
            ans = str(read_line).find("FAIL")
            if ans != -1:
                # Found a failure in this test case
                failed_test = 1     # Setting this to one to let the system know that a failure has been detected

            ans = str(read_line).find("</test>")    # Checking for the end of the test tag
            if ans >= 0:
                # Finished searching through this specific test tag
                test_tag = 0
                status_tag = 0
                failed_status_list.append(status_str)
                status_str = ""

        if failed_test == 1:
            failed_test = 0     # Resetting the failed test flag
            add_test_case = 1   # If this stays at 1, the test case is added
            for check_existing_element in failed_test_list:
                if check_existing_element == test_name:
                    # name already exists
                    add_test_case = 0
            if add_test_case == 1:   # It doesn't already exist
                failed_test_list.append(test_name)

    # return failed_test_list, failed_status_list
    return sanitise_names(failed_test_list), sanitise_fail_message(failed_status_list)


def sanitise_fail_message(input_list):
    """
    @info:                  The infromation is passed in as a list with two large lines in it. The message is
                            essentially located between "<status info > msg </status>". This method extracts the
                            message from there
    :param input_list:
    :return:
    """

    new_list = []   # This is the list that will be returned
    msg = ""
    counter = 0

    while counter < len(input_list):
        x = input_list[counter]
        if x == "":
            input_list.remove(x)      # Removing empty indices
            counter = 0                 # Start at 0 again
        else:
            counter += 1

    for error_message in input_list:
        extracting_msg = error_message.split(">")
        try:
            msg = extracting_msg[1]
        except IndexError:
            print("Index out of range in Check Errors Robot Framework file")
        final_msg = msg.replace("</status", "")
        final_msg = final_msg.replace("\n", "")
        # Getting rid of any speechmarks because they don't like the hub utility
        final_msg = final_msg.replace("\"", "")
        new_list.append(final_msg)

    return new_list


def sanitise_names(input_list):
    """
    @info:                  Sanitises the name of the failed test cases, so uses the stuff what says "name" only
    :param input_list:
    :return:
    """
    new_list = []
    for name in input_list:
        test_name = name.split("name")[1]
        test_name = test_name[2:]
        test_name = test_name[:-3]
        new_list.append(test_name)

    return new_list


# if __name__ == '__main__':
#
#     # file_name = sys.argv[1]
#     # file_name = "../temp/output-20190814-084257.xml"
#
    # failed_test_cases, reasons = read_file_state_errors("output-20190918-085115.xml")
#
#     # failed_test_cases, reasons = read_file_state_errors("com.xml")
#     print("failed test cases are {}".format(failed_test_cases))
#     print("Failed reasons are {}".format(reasons))

