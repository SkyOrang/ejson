# ejson

Based on the C/C++ standard library, programmers can easily perform common operations such as adding, deleting, querying and modifying various fields in JSON files with excellent performance.

For detailed instructions, please refer to the wiki.

**Characteristics**

- Utilizes interfaces for simplicity and efficiency (based on [], <<, >> operator overloading)

- Excellent performance
  
- Cross-platform (based on C/C++ standard library)

**Performance testing**

Test environment: i7-12650H, Windows 11, MSVC Release (/O2)

Test content: Compare the three core indicators of ejson with nlohmann/json and RapidJSON in a pure memory environment.

Parsing query: Parse a 4KB message and extract deep nodes.

From scratch construction: Dynamically assemble a complex message containing 100 objects.

Memory consumption: The growth of physical memory fragments after extreme high-frequency construction.

EN:

<img width="1427" height="587" alt="image" src="https://github.com/user-attachments/assets/a08a0e6d-b52a-4fae-b1fe-5082047ae5b0" >

CN:

<img width="1385" height="531" alt="image" src="https://github.com/user-attachments/assets/0fe053cd-dcda-4540-b519-930155191048" />


