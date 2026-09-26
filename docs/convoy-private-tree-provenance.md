# Private behavior-tree distribution checkpoint

Reviewed September 26, 2026. This resolves the scope of the current question; it does not grant redistribution rights or change a live package.

The canonical source now matches the separately built authored export: the copied `ActivityEntityFollow.bt`, inactive `ActivityVehicleEntityLoop.bt`, and four dependent legacy world variants are retained only in private archives and frozen comparison packages. The authored `ActivityOwnedVehicleFollow.bt` remains an opt-in test backend. Normal driver prefabs have not adopted it. See [the authored source checkpoint](convoy-authored-source-checkpoint.md) for exact validation, physical evidence and remaining limits.

The historical audit in `.cache/original-follow-release-export-audit.md` identified the old factory's reachable no-lease branch. The clean export removes that binding, refuses unleased creation, omits dependent runnable legacy worlds, and regenerates the resource database. Five-configuration validation, a narrow excluded-content package audit and a bounded physical Hold/Resume comparison are now recorded. The missing-lease negative gameplay case remains unobserved. Earlier mixed-source packages remain private and unchanged; removing this dependency is not general rights certification or production acceptance.

Bohemia's [data-modding reference](https://community.bistudio.com/wiki/Arma_Reforger:Data_Modding_Basics) describes behavior-tree replacement and limited modification, without inheritance. That describes technical capabilities, not a public source license.

Section 2 of the [Workshop Terms](https://reforger.armaplatform.com/workshop-terms) permits sharing content combining game content and an author's work through Workshop, subject to the other terms. The [official IP FAQ](https://reforger.armaplatform.com/news/workshop-licenses-and-ip-faq) likewise discusses vanilla derivatives within Reforger and its Workshop. Neither establishes a general right to place a copied game tree in this public GitHub repository.

The installed `Arma Reforger Tools/Docs/License.txt` distinguishes user-created content from redistribution of the software or its parts. The [EULA FAQ](https://reforger.armaplatform.com/news/eula-faq) confirms that Tools, game and Workshop terms govern different activities. Do not apply a Workshop permission automatically to public source hosting.

Continue versioning original scripts, documentation and independent tooling. Keep the copied tree and dependent private packs local while the distribution basis is unresolved. Compare the original graph through the already reviewed ownership contract before replacing the copied control; do not treat a graph rewrite or compilation as equivalent driving evidence. Workshop publication remains outside this assignment's authorization.
