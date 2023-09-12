import unittest
from unittest.mock import MagicMock
from . import main

class TestIptables(unittest.TestCase):
    def test_append_rule(self):
        main.run = MagicMock()
        main.iptables_path = "iptables"
        iptables = main.Iptables()
        iptables.append_rule("INPUT", "-p icmp -i bf-veth2 -j DROP")
        main.run.assert_called_with("iptables -A INPUT -p icmp -i bf-veth2 -j DROP")

if __name__ == "__main__":
    unittest.main()
