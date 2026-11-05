# ejson

Based on the C/C++ standard library, programmers can easily perform common operations such as adding, deleting, querying and modifying various fields in JSON files with excellent performance.

For detailed instructions, please refer to the wiki.

**Characteristics**

- Utilizes interfaces for simplicity and efficiency (based on [], <<, >> operator overloading)

- Excellent performance
  
- Cross-platform (based on C/C++ standard library)

**Performance testing**

The test code is located in the "test" folder of the source code. Those who are interested can try running it on their own computers.

Test environment: i7-12650H, Windows 11, MSVC Release (/O2)

Test content: (Reading and Writing Test)Compare the three core indicators of ejson with nlohmann/json and RapidJSON in a pure memory environment.

Parsing query: Parse a 4KB message and extract deep nodes.

From scratch construction: Dynamically assemble a complex message containing 100 objects.

Memory consumption: The growth of physical memory fragments after extreme high-frequency construction.

EN:

<img width="1427" height="587" alt="image" src="https://github.com/user-attachments/assets/a08a0e6d-b52a-4fae-b1fe-5082047ae5b0" >

CN:

<img width="1385" height="531" alt="image" src="https://github.com/user-attachments/assets/0fe053cd-dcda-4540-b519-930155191048" />


Test content: (Read-only test)In a pure memory environment, 1 million high-frequency reads of deep nodes were simulated using 4KB messages. ejson uses a loop-
based  extraction handle (to record the absolute physical offset) for O(1) direct access; simdjson reads using its DOM interface after SIMD parsing.

EN:

<img width="1382" height="377" alt="image" src="https://github.com/user-attachments/assets/30f80b3d-a648-436c-9b01-2e2e1f02330b" />

CN:

<img width="1436" height="417" alt="image" src="https://github.com/user-attachments/assets/82d7f528-aded-4377-9246-50cbd0839f13" />






