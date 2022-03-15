# Pull Request (PR) into Code-2022

***Before making a pull request, you must:
Follow HyTech's [Github Software Guidelines](https://gtvault.sharepoint.com/:w:/r/sites/HyTechRacing2/Shared%20Documents/Electrical%20-%20All/HT06/Guidelines%20Rules%20and%20Procedures/Github%20Software%20Guidelines.docx?d=w7e0d7e36afeb47b5a935bd7a4851bceb&csf=1&web=1&e=sudzjV) to understand our development flow and pull request procedure.***

***Your PR should not cause any merge conflicts, so please merge the `main` branch into your branch before submitting. It is your responsibility to ensure that the merge does not break something before submitting. [Here](https://guides.github.com/features/mastering-markdown/) is a good resource for markdown formatting as you fill out this template. See this [example](). Feel free to ask any electrical lead if you have any questions! Delete this block when actually making your PR!***

## Code Description
*Describe the contents of the pull request. Describe what major files were changed. If you changed or added libraries, please give details on what those changes are.*

## Testing Description
*Describe how you tested the contents of the pull request. Describe how each major change was tested. Describe how you ensured functionality regressions. Link to any gathered data from testing if possible.*

## Additional Information
*Put any additional datasheets, information, and/or useful links here.*

## Checklist
- [ ] Is this code linked to a new board or board rev?
- - [ ] Is *there a PR* for that board in `circuits-2022`? If so, please pause until that PR is merged.
- [ ] If you made a change or addition or deletion to the CAN library, did you inform the Data Aq lead?
- [ ] Did you test the code in real-world conditions before submitting?
- - [ ] Did you *use CPU Speed = 720 MHz (overclock) and Optimize = Fastest* when testing with Teensy 4.x?
- - [ ] Did you *use CPU Speed = 144 MHz (overclock) and Optimize = Fastest with LTO* when testing with Teensy 3.5?
- - [ ] Did you *use CPU Speed = 120 MHz (overclock) and Optimize = Fastest with LTO* when testing with Teensy 3.2?
- - [ ] Did you *use Teensyduino 1.56 with Arduino 1.8.19* when testing with the latest libraries from `main`?
- [ ] Did you pull `main` into your branch?
- - [ ] Did you *check for merge conflicts*?
- - [ ] Did you *resolve* any that occurred? ***If you are having trouble or are confused, contact a lead!***
- [ ] Did you fill out the above template?
- [ ] Did you assign the right people for review (on the right)?
