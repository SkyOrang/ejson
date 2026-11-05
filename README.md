# ejson

Based on the C/C++ standard library, programmers can easily perform common operations such as adding, deleting, querying and modifying various fields in JSON files with excellent performance.

For detailed instructions, please refer to the wiki.

**Characteristics**

- Utilizes interfaces for simplicity and efficiency (based on [], <<, >> operator overloading)

- Excellent performance
  
- Cross-platform (based on C/C++ standard library)

**Performance testing**

Test environment: i7-12650H, Windows 11, MSVC Release (/O2), pure memory operation

Performance indicators: Achieve ultra-fast packet assembly of 5KB messages in 4 µs, and parsing of 4KB messages in only 11 µs; Invented handle cache mechanism, which significantly reduces the time for deep modifications by 232 times (down to 430 ns), and passed full-scale fuzzy testing (with zero crashes).

EN:

<img width="1427" height="587" alt="image" src="https://github.com/user-attachments/assets/a08a0e6d-b52a-4fae-b1fe-5082047ae5b0" 

CN:

<img width="1385" height="531" alt="image" src="https://github.com/user-attachments/assets/0fe053cd-dcda-4540-b519-930155191048" />


